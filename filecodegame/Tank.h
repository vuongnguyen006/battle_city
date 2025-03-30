
#ifndef TANK_H
#define TANK_H
#include <SDL.h>
#include <vector>
#include "Bullet.h"
#include "Wall.h"
#include "Constants.h"

class Game;

class Tank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    std::vector<Bullet> bullets;
    SDL_Texture* tanks;
    int direction;
    Game* game;

    Tank(int startX, int startY, SDL_Texture* texture, Game* gamePtr);
    void move(int dx, int dy, const std::vector<Wall>& walls);
    void shoot();
    void update(std::vector<Wall>& walls);
    void render(SDL_Renderer* renderer);
};

#endif
