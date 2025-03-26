
#ifndef TANK_H
#define TANK_H
#include <SDL.h>
#include <vector>
#include "Bullet.h"
#include "Wall.h"
#include "Constants.h"

class Tank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    std::vector<Bullet> bullets;
    SDL_Texture* tanks;
    int direction;

    Tank(int startX, int startY, SDL_Texture* texture);
    void move(int dx, int dy, const std::vector<Wall>& walls);
    void shoot();
    void update();
    void render(SDL_Renderer* renderer);
};

#endif
