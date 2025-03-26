
#include "Bullet.h"

Bullet::Bullet(int startX, int startY, int dX, int dY) {
    x = startX;
    y = startY;
    dirX = dX;
    dirY = dY;
    active = true;
    rect = {x, y, 10, 10};
}

void Bullet::update() {
    if (active) {
        x += dirX * 5;
        y += dirY * 5;
        rect.x = x;
        rect.y = y;
    }
    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
        active = false;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (active) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}
