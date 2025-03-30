#ifndef WALL_H
#define WALL_H
#include <SDL.h>
#include "Constants.h"

class Wall {
public:
    enum WallType {BRICK, STEEL};
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    bool destructible = false;
    bool destroyed = false;
    WallType type;

    Wall(int x, int y, WallType type, SDL_Renderer* renderer);
    ~Wall();
    void render(SDL_Renderer* renderer);
};

#endif
