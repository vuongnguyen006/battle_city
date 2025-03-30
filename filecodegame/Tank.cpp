#include "Tank.h"
#include "Game.h"
#include <algorithm>

Tank::Tank(int startX, int startY, SDL_Texture* texture, Game* gamePtr) {
    x = startX;
    y = startY;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = -1;
    tanks = texture;
    direction = 4; // Hướng lên mặc định
    game = gamePtr;
}

void Tank::move(int dx, int dy, const std::vector<Wall>& walls) {
    int newX = x + dx;
    int newY = y + dy;

    if (newX < 0 || newX + TILE_SIZE > SCREEN_WIDTH || newY < 0 || newY + TILE_SIZE > SCREEN_HEIGHT) return;

    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
    for (const Wall& w : walls) {
        if (w.active && SDL_HasIntersection(&newRect, &w.rect)) return;
    }

    x = newX;
    y = newY;
    rect.x = x;
    rect.y = y;
    dirX = dx;
    dirY = dy;

    if (dx > 0) direction = 5;
    else if (dx < 0) direction = 7;
    else if (dy > 0) direction = 6;
    else if (dy < 0) direction = 4;
}

void Tank::shoot() {
    int bulletX = x + TILE_SIZE / 2 - 2;
    int bulletY = y + TILE_SIZE / 2 - 2;
    int bulletDirX = 0, bulletDirY = 0;
    switch (direction) {
        case 4: bulletDirY = -1; break; // Lên
        case 5: bulletDirX = 1; break;  // Phải
        case 6: bulletDirY = 1; break;  // Xuống
        case 7: bulletDirX = -1; break; // Trái
    }
    bullets.push_back(Bullet(bulletX, bulletY, bulletDirX, bulletDirY));
}

void Tank::update(std::vector<Wall>& walls) {
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;
        bullet.update();

        for (auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                bullet.active = false;
                if (wall.destructible) wall.active = false;
                break;
            }
        }
    }

    bullets.erase(remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) { return !b.active; }),
        bullets.end());
}

void Tank::render(SDL_Renderer* renderer) {
    if (!tanks) return;
    int frameOffset = direction;
    SDL_Rect srcRect = {frameOffset * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE};
    SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

    for (auto& bullet : bullets) {
        if (bullet.active) bullet.render(renderer);
    }
}
