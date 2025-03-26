#ifndef BULLET_H
#define BULLET_H
#include <SDL.h>
#include "Constants.h"

class Bullet {
public:
    int x, y;
    int dirX, dirY;
    bool active;
    SDL_Rect rect;

    Bullet(int startX, int startY, int dX, int dY);
    void update();
    void render(SDL_Renderer* renderer);
};

#endif
