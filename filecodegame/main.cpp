#include <SDL.h>
#include <vector>
#include <iostream>
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

//Xe tang di chuyen ngau nhien
class EnemyTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    vector<Bullet> bullets;
    int moveTime; // Thời gian đổi hướng

    void shoot() {
        bullets.push_back(Bullet(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY));
    }

    EnemyTank(int startX, int startY) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = 1;
        moveTime = rand() % 100 + 50; // Ngẫu nhiên 50-150 frames
    }

    void changeDirection() {
        int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        int index = rand() % 4;
        dirX = directions[index][0];
        dirY = directions[index][1];
    }

    void move(const vector<Wall>& walls) {
        moveTime --;
        if (moveTime <= 0) {
            changeDirection();
            moveTime = rand() % 100 + 100;
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

        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
        moveTime--;
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ
        SDL_RenderFillRect(renderer, &rect);
    }
};


// 🎮 Class quản lý game
class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    vector<Wall> walls;
    vector<EnemyTank> enemies;
    Tank player;
    void shoot();

    Game() : player((MAP_WIDTH / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE) {
        running = true;
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL không khởi động được! SDL_Error: " << SDL_GetError() << endl;
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

        generateWalls();
    }

    void generateWalls() {
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
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: player.move(0, -5, walls); break;
                    case SDLK_DOWN: player.move(0, 5, walls); break;
                    case SDLK_LEFT: player.move(-5, 0, walls); break;
                    case SDLK_RIGHT: player.move(5, 0, walls); break;
                    case SDLK_SPACE: player.shoot(); break;
                }
            }
        }
    }

    void update() {
        player.update();
        for (auto& enemy : enemies) {
            enemy.move(walls);
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (auto& wall : walls) {
            wall.render(renderer);
        }

        for (auto& enemy : enemies) {
            enemy.render(renderer);
        }

        player.render(renderer);

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
