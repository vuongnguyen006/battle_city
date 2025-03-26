
#ifndef ENEMY_TANK_H
#define ENEMY_TANK_H
#include <SDL.h>
#include <vector>
#include "Bullet.h"
#include "Wall.h"
#include "Constants.h"

class EnemyTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    std::vector<Bullet> bullets;
    int moveTime;
    int shootTimer;
    bool active;
    SDL_Texture* tanks;
    int direction;
    int explosionFrame;
    int explosionCounter;
    bool exploding;

    EnemyTank(int startX, int startY, SDL_Texture* texture);
    void changeDirection();
    void shoot();
    void update(const std::vector<Wall>& walls);
    void render(SDL_Renderer* renderer);
};

#endif
