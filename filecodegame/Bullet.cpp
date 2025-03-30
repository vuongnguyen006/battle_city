#include "Bullet.h"
#include "Constants.h"

Bullet::Bullet(int startX, int startY, int dirX, int dirY) {
    x = startX;
    y = startY;
    dx = dirX * 5;
    dy = dirY * 5;
    rect = { x, y, 4, 4};  // Cỡ viên đạn
    active = true;
}

void Bullet::update() {
    x += dx;
    y += dy;
    rect.x = x;
    rect.y = y;

    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) active = false;
}

void Bullet::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Màu trắng
    SDL_RenderFillRect(renderer, &rect);
}
