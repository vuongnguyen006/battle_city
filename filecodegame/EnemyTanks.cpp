
#include "EnemyTanks.h"
#include <cstdlib>
#include <algorithm>

EnemyTank::EnemyTank(int startX, int startY, SDL_Texture* texture) {
    x = startX;
    y = startY;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = 1;
    moveTime = rand() % 100 + 50;
    shootTimer = 60; // Bắn mỗi 1 giây
    active = true;
    tanks = texture;
    direction = 2;
    explosionFrame = 0;
    explosionCounter = 0;
    exploding = false;
}

void EnemyTank::changeDirection() {
    int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    int index = rand() % 4;
    dirX = directions[index][0];
    dirY = directions[index][1];

    if (dirX > 0) direction = 1;
    else if (dirX < 0) direction = 3;
    else if (dirY > 0) direction = 2;
    else if (dirY < 0) direction = 0;
}

void EnemyTank::shoot() {
    bullets.push_back(Bullet(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY));
}

void EnemyTank::update(const std::vector<Wall>& walls) {
    if (!active) {
        if (exploding) {
            explosionCounter++;
            if (explosionCounter >= 5) {
                explosionFrame++;
                explosionCounter = 0;
            }
            if (explosionFrame >= 4) exploding = false;
        }
        return;
    }

    for (auto& bullet : bullets) {
        bullet.update();
        for (const Wall& wall : walls) {
            if (bullet.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                bullet.active = false;
                break;
            }
        }
    }
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.active; }),
        bullets.end()
    );

    shootTimer--;
    if (shootTimer <= 0) {
        shoot();
        shootTimer = 120;
    }

    moveTime--;
    if (moveTime <= 0) {
        changeDirection();
        moveTime = rand() % 100 + 50;
    }

    int newX = x + dirX * 2;
    int newY = y + dirY * 2;

    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

    bool canMove = true;
    for (const Wall& wall : walls) {
        if (SDL_HasIntersection(&newRect, &wall.rect)) {
            canMove = false;
            break;
        }
    }

    if (canMove) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }
}

void EnemyTank::render(SDL_Renderer* renderer) {
    if (active) {
        int frameOffset = direction * 1;
        SDL_Rect srcRect = {frameOffset * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
        SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

        for (auto& bullet : bullets) {
            bullet.render(renderer);
        }
    }
    else if (exploding) {
        SDL_Rect srcRect = {(8 + explosionFrame) * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE};
        SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);
    }
}
