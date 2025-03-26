#ifndef WALL_H
#define WALL_H
#include <SDL.h>
#include "Constants.h"

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;

    Wall(int x, int y, SDL_Renderer* renderer);
    ~Wall();
    void render(SDL_Renderer* renderer);
};

#endif
