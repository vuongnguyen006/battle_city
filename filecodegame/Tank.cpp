
#include "Tank.h"

Tank::Tank(int startX, int startY, SDL_Texture* texture) {
    x = startX;
    y = startY;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = -1;
    tanks = texture;
    direction = 0;
}

void Tank::move(int dx, int dy, const std::vector<Wall>& walls) {
    int newX = x + dx;
    int newY = y + dy;

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

    if (dx > 0) direction = 1;
    else if (dx < 0) direction = 3;
    else if (dy > 0) direction = 2;
    else if (dy < 0) direction = 0;
}

void Tank::shoot() {
    bullets.push_back(Bullet(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY));
}

void Tank::update() {
    for (auto& bullet : bullets) {
        bullet.update();
    }
}

void Tank::render(SDL_Renderer* renderer) {
    int frameOffset = direction * 1;
    SDL_Rect srcRect = {frameOffset * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE};
    SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(renderer, tanks, &srcRect, &dstRect);

    for (auto& bullet : bullets) {
        bullet.render(renderer);
    }
}
