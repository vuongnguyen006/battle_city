#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>

class Bullet {
public:
    int x, y;
    int dx, dy;
    SDL_Rect rect;
    bool active;

    Bullet(int startX, int startY, int dirX, int dirY);

    void update();
    void render(SDL_Renderer* renderer);
};

#endif
