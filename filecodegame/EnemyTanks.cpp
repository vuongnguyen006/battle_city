#include "EnemyTanks.h"
#include "Wall.h"
#include "Game.h"
#include <SDL.h>
#include <cstdlib>
#include <algorithm>

EnemyTank::EnemyTank(int startX, int startY, SDL_Texture* texture, Game* gamePtr) {
    x = startX;
    y = startY;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = 1;
    moveTime = rand() % 60 + 60;
    shootTimer = 60;
    active = true;
    tanks = texture;
    direction = 2; // xuống
    explodeFrame = 0;
    exploding = false;
    game = gamePtr;
}

void EnemyTank::changeDirection() {
    int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    int index = rand() % 4;
    dirX = directions[index][0];
    dirY = directions[index][1];

    // Cập nhật direction phù hợp
    if (dirX == 1) direction = 1;
    else if (dirX == -1) direction = 3;
    else if (dirY == 1) direction = 2;
    else if (dirY == -1) direction = 0;
}

void EnemyTank::shoot() {
    if (!active) return;

    int bulletX = x + TILE_SIZE / 2 - 2;
    int bulletY = y + TILE_SIZE / 2 - 2;
    int bulletDirX = 0, bulletDirY = 0;

    switch (direction) {
        case 0: bulletDirY = -1; break; // lên
        case 1: bulletDirX = 1; break;  // phải
        case 2: bulletDirY = 1; break;  // xuống
        case 3: bulletDirX = -1; break; // trái
        default: bulletDirY = 1; break;
    }

    bullets.emplace_back(bulletX, bulletY, bulletDirX, bulletDirY);
}

void EnemyTank::update(const std::vector<Wall>& walls) {
    if (!active) {
        if (exploding) {
            explodeFrame++;
            if (explodeFrame >= 60) {
                exploding = false;
            }
        }
        return;
    }

    // Cập nhật đạn
    for (auto& bullet : bullets) {
        if (bullet.active) bullet.update();
    }

    // Di chuyển
    moveTime--;
    if (moveTime <= 0) {
        changeDirection();
        moveTime = rand() % 60 + 60;
    }

    int newX = x + dirX * 2;
    int newY = y + dirY * 2;
    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
    bool canMove = true;

    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
            canMove = false;
            break;
        }
    }

    if (canMove) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    } else {
        changeDirection();
        moveTime = rand() % 60 + 60;
    }

    // Bắn ngẫu nhiên
    if (rand() % 100 < 3) {
        shoot();
    }

    // Xoá đạn không còn hoạt động
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) { return !b.active; }), bullets.end());
}

void EnemyTank::render(SDL_Renderer* renderer) {
    if (!active) {
        if (exploding) {
            SDL_Rect srcRect = {(8 + explodeFrame) * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE};
            SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);
        }
        return;
    }

    SDL_Rect srcRect = {direction * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE};
    SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

    for (auto& bullet : bullets) {
        if (bullet.active) bullet.render(renderer);
    }
}
