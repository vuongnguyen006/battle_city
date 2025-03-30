#include "Game.h"
#include <iostream>
#include <fstream>
#include <algorithm>

int totalEnemiesSpawned = 0;
const int MAX_TOTAL_ENEMIES = 20;

Game::Game() : player1((MAP_WIDTH / 4) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks, this) {
    gameState = MENU;
    running = true;
    window = nullptr;
    renderer = nullptr;
    font = nullptr;
    tanks = nullptr;
    eagleTexture = nullptr;
    wallBreakSound = nullptr;
    menuSound = nullptr;
    score = 0;
    highScore = 0;
    selectedMenuItem = 0;
    lives = 10;
    protectedZoneActive = true;

    // Khởi tạo SDL và các thư viện
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() == -1 || IMG_Init(IMG_INIT_PNG) == 0 || Mix_Init(MIX_INIT_MP3) == 0) {
        std::cerr << "Không thể khởi tạo SDL hoặc các thư viện: " << SDL_GetError() << " | " << TTF_GetError() << " | " << IMG_GetError() << " | " << Mix_GetError() << std::endl;
        running = false;
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Không thể khởi tạo audio! Mix_Error: " << Mix_GetError() << std::endl;
        running = false;
        return;
    }

    window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Không thể tạo cửa sổ! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Không thể tạo renderer! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    font = TTF_OpenFont("PressStart2P.ttf", 16);
    if (!font) {
        font = TTF_OpenFont("DejaVuSans.ttf", 16);
        if (!font) {
            std::cerr << "Không thể tải font! TTF_Error: " << TTF_GetError() << std::endl;
            running = false;
            return;
        }
    }

    SDL_Surface* surface = IMG_Load("tanks.png");
    if (!surface) {
        std::cerr << "Không thể tải texture tanks! IMG_Error: " << IMG_GetError() << std::endl;
        running = false;
        return;
    }
    tanks = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!tanks) {
        std::cerr << "Không thể tạo texture từ tanks! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    surface = IMG_Load("bird.png");
    if (!surface) {
        std::cerr << "Không tải được EAGLE! IMG_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }
    eagleTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!eagleTexture) {
        std::cerr << "Không thể tạo texture từ eagle! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    wallBreakSound = Mix_LoadWAV("break.mp3");
    if (!wallBreakSound) {
        std::cerr << "Không thể tải break.mp3! Mix_Error: " << Mix_GetError() << std::endl;
    }

    menuSound = Mix_LoadMUS("menuSound.mp3");
    if (!menuSound) {
        std::cerr << "Không thể tải Menu_game_sound.mp3! Mix_Error: " << Mix_GetError() << std::endl;
    }else {
        Mix_PlayMusic(menuSound, -1);
    }

    // Đọc high score
    std::ifstream inFile("highscore.txt");
    if (inFile.is_open()) {
        inFile >> highScore;
        inFile.close();
    } else {
        std::ofstream outFile("highscore.txt");
        if (outFile.is_open()) {
            outFile << 0;
            outFile.close();
        } else {
            std::cerr << "Không thể tạo file highscore.txt!" << std::endl;
        }
        highScore = 0;
    }

    // Cập nhật texture cho player1 sau khi tanks được tạo
    //player1 = Tank((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks, this);
    // Khởi tạo spawnX và spawnY
    spawnX = {0, (MAP_WIDTH / 2) * TILE_SIZE, (MAP_WIDTH - 1) * TILE_SIZE};
    spawnY = {0, TILE_SIZE, 2 * TILE_SIZE};
    generateWalls();
    spawnEnemies();
}

Game::~Game() {
    if (font) TTF_CloseFont(font);
    if (tanks) SDL_DestroyTexture(tanks);
    if (eagleTexture) SDL_DestroyTexture(eagleTexture);
    if (wallBreakSound) Mix_FreeChunk(wallBreakSound);
    if (menuSound) Mix_FreeMusic(menuSound);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::run() {
    while (running) {
        handleEvents();
        update();
        render();
        SDL_Delay(16); // ~60 FPS
    }
}

void Game::loadMap() {
    std::ifstream mapFile("map.txt");
    if (!mapFile.is_open()) {
        std::cerr << "Không thể mở file map.txt!" << std::endl;
        running = false;
        return;
    }

    std::string line;
    int y = 0;
    while (std::getline(mapFile, line) && y < MAP_HEIGHT) {
        if (line.length() != MAP_WIDTH) {
            std::cerr << "Dòng " << y + 1 << " trong map.txt không có đúng " << MAP_WIDTH << " ký tự!" << std::endl;
            continue;
        }
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = line[x];
        }
        y++;
    }
    mapFile.close();
}

void Game::generateWalls() {
    walls.clear();
    loadMap();
    if (!running) return;

    bool foundProtectedZone = false;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char tile = map[y][x];
            int posX = x * TILE_SIZE;
            int posY = y * TILE_SIZE;

            if (tile == '1') {
                walls.push_back(Wall(posX, posY, Wall::BRICK, renderer));
            } else if (tile == '3') {
                walls.push_back(Wall(posX, posY, Wall::STEEL, renderer));
            } else if (tile == '5' && !foundProtectedZone) {
                protectedZone = {posX, posY, TILE_SIZE * 2, TILE_SIZE * 2};
                foundProtectedZone = true;
            }
        }
    }

    if (!foundProtectedZone) {
        std::cerr << "Bản đồ không có vùng bảo vệ (ký tự 5)!" << std::endl;
        running = false;
    }
}

void Game::spawnEnemies() {
     int currentEnemies = enemies.size();
     int enemiesToSpawn = 4 - currentEnemies;
     if (enemiesToSpawn <= 0) return;

     for (int i = 0; i < enemiesToSpawn; i++) {
         bool valid;
         int ex, ey;
         int attempts = 0;
         do {
             valid = true;
             ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
             ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
             SDL_Rect newRect = {ex, ey, TILE_SIZE, TILE_SIZE};
             for (const auto& wall : walls) {
                 if (SDL_HasIntersection(&newRect, &wall.rect)) {
                     valid = false;
                     break;
                 }
             }
             if (SDL_HasIntersection(&newRect, &player1.rect)) {
                 valid = false;
             }
             for (const auto& enemy : enemies) {
                 if (SDL_HasIntersection(&newRect, &enemy.rect)) {
                     valid = false;
                     break;
                 }
             }
             attempts++;
             if (attempts > 100) {
                 std::cerr << "Không thể tìm vị trí hợp lệ cho enemy " << i << std::endl;
                 break;
             }
         } while (!valid);
         if (valid) {
             enemies.push_back(EnemyTank(ex, ey, tanks, this));
            totalEnemiesSpawned++;
         }
     }
 }

void Game::resetGame() {
    player1 = Tank((MAP_WIDTH / 4) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks, this);
    enemies.clear();
    walls.clear();
    loadMap();
    generateWalls();
    spawnEnemies();
    gameState = PLAYING;
    score = 0;
    lives = 10;
    protectedZoneActive = true;

    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
        std::cout << "Tắt nhạc nền khi vào trạng thái chơi." << std::endl;
    }

    // Ghi high score khi reset game
    std::ofstream outFile("highscore.txt");
    if (outFile.is_open()) {
        outFile << highScore;
        outFile.close();
    } else {
        std::cerr << "Không thể ghi file highscore.txt!" << std::endl;
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (gameState == MENU) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;
                        break;
                    case SDLK_DOWN:
                        selectedMenuItem = (selectedMenuItem + 1) % 2;
                        break;
                    case SDLK_RETURN:
                        if (selectedMenuItem == 0) resetGame();
                        else if (selectedMenuItem == 1) running = false;
                        break;
                }
            } else if (gameState == PLAYING) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: player1.move(0, -5, walls); break;
                    case SDLK_DOWN: player1.move(0, 5, walls); break;
                    case SDLK_LEFT: player1.move(-5, 0, walls); break;
                    case SDLK_RIGHT: player1.move(5, 0, walls); break;
                    case SDLK_SPACE: player1.shoot(); break;
                    case SDLK_p:
                        gameState = PAUSED;
                        if (menuSound && !Mix_PlayingMusic()) {
                            Mix_PlayMusic(menuSound, -1);
                        }
                        break;
                }
            } else if (gameState == PAUSED) {
                switch (event.key.keysym.sym) {
                    case SDLK_p:
                    gameState = PLAYING;
                    if (Mix_PlayingMusic()) {
                        Mix_HaltMusic();
                    }
                    break;
                }
            }
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                gameState = MENU;
                if (menuSound && !Mix_PlayingMusic()) {
                    Mix_PlayMusic(menuSound, -1);
                    std::cout << "Phát nhạc nền khi quay lại menu (nhấn ESC)." << std::endl;
                }
            }
        }
    }
}

void Game::update() {
    if (gameState != PLAYING) {
        if (gameState == MENU && !Mix_PlayingMusic() && menuSound) {
            Mix_PlayMusic(menuSound, -1);
            std::cout << "Phát nhạc nền khi quay lại menu." << std::endl;
        }
        return;
    }

    player1.update(walls);
    for (auto& enemy : enemies) {
        enemy.update(walls);
    }

    // Đạn của người chơi
    for (auto& bullet : player1.bullets) {
        if (!bullet.active) continue;
        // Không gọi bullet.update() ở đây vì đã gọi trong Tank::update()

        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            if (SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                enemy.active = false;
                enemy.exploding = true;
                bullet.active = false;
                score += 10;
                if (score > highScore) highScore = score;
                break;
            }
        }

        if (!bullet.active) continue;
        for (auto& wall : walls) {
            if (!wall.active || !wall.destructible) continue;
            if (SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                wall.active = false;
                bullet.active = false;
                int mapX = wall.x / TILE_SIZE;
                int mapY = wall.y / TILE_SIZE;
                if (mapY >= 0 && mapY < MAP_HEIGHT && mapX >= 0 && mapX < MAP_WIDTH) {
                    map[mapY][mapX] = '0';
                }
                if (wallBreakSound) Mix_PlayChannel(-1, wallBreakSound, 0);
                break;
            }
        }
    }

    // Đạn của kẻ thù
    for (auto& enemy : enemies) {
        for (auto& bullet : enemy.bullets) {
            if (!bullet.active) continue;

            if (SDL_HasIntersection(&bullet.rect, &player1.rect)) {
                lives --;
                bullet.active = false;
                if (lives <= 0){
                    resetGame();
                }else {
                    player1 = Tank((MAP_WIDTH / 4) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks, this);
                }
                return;
            }
            if (protectedZoneActive && SDL_HasIntersection(&bullet.rect, &protectedZone)) {
                protectedZoneActive = false;
                resetGame();
                return;
            }

            for (auto& wall : walls) {
                if (!wall.active) continue;
                if (SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    bullet.active = false;
                    if (wall.destructible){
                        wall.active = false;
                        int mapX = wall.x / TILE_SIZE;
                        int mapY = wall.y / TILE_SIZE;
                        if (mapY >= 0 && mapY < MAP_HEIGHT && mapX >= 0 && mapX < MAP_WIDTH) {
                            map[mapY][mapX] = '0';
                        }
                        if (wallBreakSound) {
                            Mix_PlayChannel(-1, wallBreakSound, 0);
                        }
                    }
                    break;
                }
            }
        }
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active && !e.exploding; }), enemies.end());
    player1.bullets.erase(std::remove_if(player1.bullets.begin(), player1.bullets.end(), [](Bullet& b) { return !b.active; }), player1.bullets.end());
    walls.erase(std::remove_if(walls.begin(), walls.end(), [](Wall& w) { return !w.active; }), walls.end());

    if (enemies.size() < 4) {
        std::cout << "Số lượng kẻ thù hiện tại: " << enemies.size() << ". Gọi spawnEnemies()." << std::endl;
        spawnEnemies();
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gameState == MENU) {
        SDL_Rect srcRect = {0, 260, 400, 80};
        SDL_Rect dstRect = {SCREEN_WIDTH / 2 - 100, 50, 200, 40};
        SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

        SDL_Color textColor = {255, 255, 255, 255};
        const char* menuItems[] = {"Start", "Exit"};
        for (int i = 0; i < 2; i++) {
            SDL_Surface* surface = TTF_RenderText_Solid(font, menuItems[i], textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_Rect textRect = {SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 - 30 + i * 40, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &textRect);

                if (i == selectedMenuItem) {
                    SDL_Rect iconSrc = {0, 0, SPRITE_SIZE, SPRITE_SIZE};
                    SDL_Rect iconDst = {SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 30 + i * 40, 16, 16};
                    SDL_RenderCopy(renderer, tanks, &iconSrc, &iconDst);
                }
                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
            }
        }
    } else if (gameState == PLAYING || gameState == PAUSED) {
        // Vẽ các thành phần game ngay cả khi tạm dừng để giữ nguyên khung cảnh
        for (auto& wall : walls) wall.render(renderer);
        for (auto& enemy : enemies) enemy.render(renderer);
        player1.render(renderer);

        if (protectedZoneActive) {
            SDL_Rect srcRect = {0, 0, 16, 16};
            SDL_Rect dstRect = {protectedZone.x, protectedZone.y, TILE_SIZE * 2, TILE_SIZE * 2};
            SDL_RenderCopy(renderer, eagleTexture, &srcRect, &dstRect);
        }

        SDL_Color textColor = {255, 255, 255, 255};
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (scoreSurface) {
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = {750, 20, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
            SDL_FreeSurface(scoreSurface);
        }

        std::string highScoreText = "High Score: " + std::to_string(highScore);
        SDL_Surface* highScoreSurface = TTF_RenderText_Solid(font, highScoreText.c_str(), textColor);
        if (highScoreSurface) {
            SDL_Texture* highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
            SDL_Rect highScoreRect = {750, 50, highScoreSurface->w, highScoreSurface->h};
            SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
            SDL_DestroyTexture(highScoreTexture);
            SDL_FreeSurface(highScoreSurface);
        }

        std::string livesText = "Lives: " + std::to_string(lives);
        SDL_Surface* livesSurface = TTF_RenderText_Solid(font, livesText.c_str(), textColor);
        if (livesSurface) {
            SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(renderer, livesSurface);
            SDL_Rect livesRect = {750, 80, livesSurface->w, livesSurface->h};  // Dưới High Score
            SDL_RenderCopy(renderer, livesTexture, NULL, &livesRect);
            SDL_DestroyTexture(livesTexture);
            SDL_FreeSurface(livesSurface);
        }

        if (gameState == PAUSED) {
            SDL_Surface* pauseSurface = TTF_RenderText_Solid(font, "Paused", textColor);
            if (pauseSurface) {
                SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface);
                SDL_Rect pauseRect = {SCREEN_WIDTH / 2 - pauseSurface->w / 2, SCREEN_HEIGHT / 2 - pauseSurface->h / 2, pauseSurface->w, pauseSurface->h};
                SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseRect);
                SDL_DestroyTexture(pauseTexture);
                SDL_FreeSurface(pauseSurface);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
