#include "Game.h"
#include <iostream>
#include <fstream>
#include <algorithm>

Game::Game() : player1((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, nullptr) {
    gameState = MENU;
    running = true;
    font = nullptr;
    score = 0;
    highScore = 0;
    selectedMenuItem = 0;
    tanks = nullptr;
    protectedZoneActive = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL không khởi động được! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf không khởi động được! TTF_Error: " << TTF_GetError() << std::endl;
        running = false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "SDL_image không khởi động được! IMG_Error: " << IMG_GetError() << std::endl;
        running = false;
    }

    window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Không thể tạo cửa sổ! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Không thể tạo renderer! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
    }

    font = TTF_OpenFont("PressStart2P.ttf", 16);
    if (!font) {
        std::cerr << "Không thể tải font! TTF_Error: " << TTF_GetError() << std::endl;
        font = TTF_OpenFont("DejaVuSans.ttf", 16);
        if (!font) {
            std::cerr << "Không thể tải font mặc định! TTF_Error: " << TTF_GetError() << std::endl;
            running = false;
        }
    }

    SDL_Surface* surface = IMG_Load("tanks.png");
    if (!surface) {
        std::cerr << "Không thể tải texture tanks! IMG_Error: " << IMG_GetError() << std::endl;
        running = false;
    }
    tanks = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!tanks) {
        std::cerr << "Không thể tạo texture từ tanks! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
    }

    std::ifstream inFile("highscore.txt");
    if (inFile.is_open()) {
        inFile >> highScore;
        inFile.close();
    } else {
        std::ofstream outFile("highscore.txt");
        if (outFile.is_open()) {
            outFile << 0;
            outFile.close();
            highScore = 0;
        } else {
            std::cerr << "Không thể tạo file highscore.txt!" << std::endl;
            highScore = 0;
        }
    }

    player1 = Tank((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks);
    generateWalls();
    spawnEnemies();
}

Game::~Game() {
    if (font) TTF_CloseFont(font);
    if (tanks) SDL_DestroyTexture(tanks);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run() {
    while (running) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

void Game::generateWalls() {
    walls.clear();

    std::ifstream mapFile("map.txt");
    if (!mapFile.is_open()) {
        std::cerr << "Không thể mở file map.txt!" << std::endl;
        return;
    }
    std::string line;
    int y = 0;
    bool foundProtectedZone = false;
    while (std::getline(mapFile, line)) {
        for (int x = 0; x < 30; x++) {
            char tile = line[x];
            int posX = x * TILE_SIZE;
            int posY = y * TILE_SIZE;

            if (tile == '1' || tile == '3') {
                walls.push_back(Wall(posX, posY, renderer));
            }
            else if (tile == '5' && !foundProtectedZone) {
                protectedZone = {posX, posY, TILE_SIZE * 2, TILE_SIZE * 2};
                foundProtectedZone = true;
            }
        }
        y++;
    }
    mapFile.close();
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
            enemies.push_back(EnemyTank(ex, ey, tanks));
        }
    }
}

void Game::resetGame() {
    player1 = Tank((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, tanks);
    enemies.clear();
    spawnEnemies();
    gameState = PLAYING;
    score = 0;
    protectedZoneActive = true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (gameState == MENU) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;
                        break;
                    case SDLK_DOWN:
                        selectedMenuItem = (selectedMenuItem + 1) % 2;
                        break;
                    case SDLK_RETURN:
                        if (selectedMenuItem == 0) {
                            resetGame();
                        }
                        else if (selectedMenuItem == 1) {
                            running = false;
                        }
                        break;
                }
            }
            else if (gameState == PLAYING) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: player1.move(0, -5, walls); break;
                    case SDLK_DOWN: player1.move(0, 5, walls); break;
                    case SDLK_LEFT: player1.move(-5, 0, walls); break;
                    case SDLK_RIGHT: player1.move(5, 0, walls); break;
                    case SDLK_SPACE: player1.shoot(); break;
                }
            }
            if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }
    }
}

void Game::update() {
    if (gameState != PLAYING) return;

    player1.update();
    for (auto& enemy : enemies) {
        enemy.update(walls);
    }

    for (auto& bullet : player1.bullets) {
        if (!bullet.active) continue;
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            if (SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                enemy.active = false;
                enemy.exploding = true;
                bullet.active = false;
                score += 10;
                if (score > highScore) {
                    highScore = score;
                    std::ofstream outFile("highscore.txt");
                    if (outFile.is_open()) {
                        outFile << highScore;
                        outFile.close();
                    } else {
                        std::cerr << "Không thể ghi file highscore.txt!" << std::endl;
                    }
                }
            }
        }
    }

    for (auto& enemy : enemies) {
        for (auto& bullet : enemy.bullets) {
            if (bullet.active && SDL_HasIntersection(&bullet.rect, &player1.rect)) {
                resetGame();
                return;
            }
            if (protectedZoneActive && bullet.active && SDL_HasIntersection(&bullet.rect, &protectedZone)) {
                protectedZoneActive = false;
                resetGame();
                return;
            }
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active && !e.exploding; }),
        enemies.end()
    );

    player1.bullets.erase(
        std::remove_if(player1.bullets.begin(), player1.bullets.end(), [](Bullet& b) { return !b.active; }),
        player1.bullets.end()
    );

    if (enemies.size() < 4) {
        spawnEnemies();
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gameState == MENU) {
        SDL_Rect srcRect = {0, 80, 80, 16};
        SDL_Rect dstRect = {SCREEN_WIDTH / 2 - 80, 50, 160, 32};
        SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

        SDL_Color textColor = {255, 255, 255, 255};
        const char* menuItems[] = {"1 Player", "Exit"};
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
    }
    else {
        for (auto& wall : walls) {
            wall.render(renderer);
        }
        for (auto& enemy : enemies) {
            enemy.render(renderer);
        }
        player1.render(renderer);

        if (protectedZoneActive) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &protectedZone);
        }

        SDL_Color textColor = {255, 255, 255, 255};
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (scoreSurface) {
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
            SDL_FreeSurface(scoreSurface);
        }

        std::string highScoreText = "High Score: " + std::to_string(highScore);
        SDL_Surface* highScoreSurface = TTF_RenderText_Solid(font, highScoreText.c_str(), textColor);
        if (highScoreSurface) {
            SDL_Texture* highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
            SDL_Rect highScoreRect = {10, 30, highScoreSurface->w, highScoreSurface->h};
            SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
            SDL_DestroyTexture(highScoreTexture);
            SDL_FreeSurface(highScoreSurface);
        }
    }

    SDL_RenderPresent(renderer);
}
