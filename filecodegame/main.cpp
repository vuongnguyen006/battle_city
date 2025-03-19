#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdlib> // Cho rand()
#include <ctime>   // Cho time()

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

// 🚧 Class tường gạch
class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;

    Wall(int startX, int startY) {
        x = startX;
        y = startY;
        active = true;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // Màu nâu
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

// 💥 Class đạn
class Bullet {
public:
    int x, y;
    int dirX, dirY;
    bool active;
    SDL_Rect rect;

    Bullet(int startX, int startY, int dX, int dY) {
        x = startX;
        y = startY;
        dirX = dX;
        dirY = dY;
        active = true;
        rect = {x, y, 10, 10};
    }

    void update() {
        if (active) {
            x += dirX * 10;
            y += dirY * 10;
            rect.x = x;
            rect.y = y;
        }
        if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

// 🚜 Class xe tăng
class Tank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    vector<Bullet> bullets;

    Tank(int startX, int startY) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
    }

    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

        // Kiểm tra va chạm với tường
        for (const Wall& w : walls) {
            if (w.active && SDL_HasIntersection(&newRect, &w.rect)) return;
        }

        // Di chuyển nếu không va chạm
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
        dirX = dx;
        dirY = dy;
    }

    void shoot() {
        bullets.push_back(Bullet(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY));
    }

    void update() {
        for (auto& bullet : bullets) {
            bullet.update();
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        for (auto& bullet : bullets) {
            bullet.render(renderer);
        }
    }
};

//Xe tang di chuyen ngau nhien va ban nguoi choi
class EnemyTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    vector<Bullet> bullets;
    int moveTime;
    int shootTimer;
    bool active;

    EnemyTank(int startX, int startY) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = 1;
        moveTime = rand() % 100 + 50;
        shootTimer = rand() % 60 + 30;
        active = true;
    }

    void changeDirection() {
        int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        int index = rand() % 4;
        dirX = directions[index][0];
        dirY = directions[index][1];
    }

    void shoot() {
        bullets.push_back(Bullet(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY));
    }

    bool canSeePlayer(const Tank& player) {
        if (dirX != 0) {
            if (abs(player.y - y) < TILE_SIZE) {
                if (dirX > 0 && player.x > x) return true;
                if (dirX < 0 && player.x < x) return true;
            }
        }
        if (dirY != 0) {
            if (abs(player.x - x) < TILE_SIZE) {
                if (dirY > 0 && player.y > y) return true;
                if (dirY < 0 && player.y < y) return true;
            }
        }
        return false;
    }

    void update(const Tank& player, const vector<Wall>& walls) {
        if (!active) return;

        for (auto& bullet : bullets) {
            bullet.update();
        }
        bullets.erase(
            remove_if(bullets.begin(), bullets.end(),
                [](Bullet& b) { return !b.active; }),
            bullets.end()
        );

        shootTimer--;
        if (canSeePlayer(player) && shootTimer <= 0) {
            shoot();
            shootTimer = 60;
        }

        moveTime--;
        if (moveTime <= 0) {
            changeDirection();
            moveTime = rand() % 100 + 50;
        }

        int newX = x + dirX * 2;
        int newY = y + dirY * 2;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

        for (const Wall& wall : walls) {
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
                changeDirection();
                return;
            }
        }

        if (SDL_HasIntersection(&newRect, &player.rect)) {
            changeDirection();
            return;
        }

        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);

            for (auto& bullet : bullets) {
                bullet.render(renderer);
            }
        }
    }
};


// 🎮 Class quản lý game
class Game {
public:
    enum GameState { PLAYING, GAME_OVER, WIN };
    GameState gameState;
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    vector<Wall> walls;
    vector<EnemyTank> enemies;
    Tank player;
    TTF_Font* font; // Biến để lưu font

    Game() : player((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE) {
        gameState = PLAYING;
        running = true;
        font = nullptr;

        // Khởi tạo SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL không khởi động được! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }

        // Khởi tạo SDL_ttf
        if (TTF_Init() == -1) {
            cerr << "SDL_ttf không khởi động được! TTF_Error: " << TTF_GetError() << endl;
            running = false;
        }

        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Không thể tạo cửa sổ! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Không thể tạo renderer! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }

        // Khởi tạo font
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24); // Đường dẫn font trên Ubuntu
        if (!font) {
            cerr << "Không thể tải font! TTF_Error: " << TTF_GetError() << endl;
            running = false;
        }

        generateWalls();
        spawnEnemies();
    }

    void generateWalls() {
        // Thêm tường bao quanh map
        // Tường trên và dưới
        for (int x = 0; x < MAP_WIDTH; x++) {
            walls.push_back(Wall(x * TILE_SIZE, 0)); // Tường trên
            walls.push_back(Wall(x * TILE_SIZE, (MAP_HEIGHT - 1) * TILE_SIZE)); // Tường dưới
        }
        // Tường trái và phải
        for (int y = 0; y < MAP_HEIGHT; y++) {
            walls.push_back(Wall(0, y * TILE_SIZE)); // Tường trái
            walls.push_back(Wall((MAP_WIDTH - 1) * TILE_SIZE, y * TILE_SIZE)); // Tường phải
        }

        // Tường khác trong map (giữ nguyên từ trước)
        for (int i = 3; i < MAP_HEIGHT - 3; i += 2) {
            for (int j = 3; j < MAP_WIDTH - 3; j += 2) {
                walls.push_back(Wall(j * TILE_SIZE, i * TILE_SIZE));
            }
        }
    }

    void spawnEnemies() {
        for (int i = 0; i < 3; i++) { // 3 xe tăng địch
            bool valid;
            int ex, ey;
            do {
                valid = true;
                ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
                ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;

                // Kiểm tra xem vị trí có trùng với tường không
                SDL_Rect newRect = {ex, ey, TILE_SIZE, TILE_SIZE};  // Tạo biến SDL_Rect trước
                for (const auto& wall : walls) {
                    if (SDL_HasIntersection(&newRect, &wall.rect)) {  // Truyền địa chỉ hợp lệ
                        valid = false;
                        break;
                    }
                }
            } while (!valid);
            enemies.push_back(EnemyTank(ex, ey));
        }
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (gameState == PLAYING) {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP: player.move(0, -5, walls); break;
                        case SDLK_DOWN: player.move(0, 5, walls); break;
                        case SDLK_LEFT: player.move(-5, 0, walls); break;
                        case SDLK_RIGHT: player.move(5, 0, walls); break;
                        case SDLK_SPACE: player.shoot(); break;
                    }
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }
    }

    void update() {
        if (gameState != PLAYING) return;

        player.update();
        for (auto& enemy : enemies) {
            enemy.update(player, walls);
        }

        for (auto& bullet : player.bullets) {
            if (!bullet.active) continue;
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                    enemy.active = false;
                    bullet.active = false;
                }
            }
        }

        for (auto& enemy : enemies) {
            for (auto& bullet : enemy.bullets) {
                if (bullet.active && SDL_HasIntersection(&bullet.rect, &player.rect)) {
                    gameState = GAME_OVER;
                    return;
                }
            }
        }

        enemies.erase(
            remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active; }),
            enemies.end()
        );

        player.bullets.erase(
            remove_if(player.bullets.begin(), player.bullets.end(), [](Bullet& b) { return !b.active; }),
            player.bullets.end()
        );

        if (enemies.empty()) {
            gameState = WIN;
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (gameState == PLAYING) {
            for (auto& wall : walls) {
                wall.render(renderer);
            }
            for (auto& enemy : enemies) {
                enemy.render(renderer);
            }
            player.render(renderer);
        }
        else if (gameState == WIN) {
            //Tạo màu viền cho Win
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect winRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3};
            SDL_RenderFillRect(renderer, &winRect);

            //YOU WIN xuất hiện giữa màn hình khi chiến thắng
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(font, "YOU WIN!", textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 10, 100, 20};
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        else if (gameState == GAME_OVER) {
            //Tạo màu nền cho GAME OVER
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect loseRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3};
            SDL_RenderFillRect(renderer, &loseRect);

            //GAME OVER màu trắng xuất hiện giữa màn hình
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(font, "GAME OVER", textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 10, 120, 20};
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }

        SDL_RenderPresent(renderer);
    }

    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);
        }
    }

    ~Game() {
        if (font) {
            TTF_CloseFont(font);
        }
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    srand(time(0));
    Game game;
    if (game.running) {
        game.run();
    }
    return 0;
}
