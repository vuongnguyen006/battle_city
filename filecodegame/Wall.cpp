
#include "Wall.h"
#include <iostream>
#include <SDL_image.h>

Wall::Wall(int x, int y, SDL_Renderer* renderer) {
    this->x = x;
    this->y = y;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    active = true;

    SDL_Surface* surface = IMG_Load("brick.png");
    if (!surface) {
        std::cerr << "Không thể tải wall.png! IMG_Error: " << IMG_GetError() << std::endl;
        texture = nullptr;
    } else {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Không thể tạo texture từ wall.png! SDL_Error: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Loaded wall.png successfully at (" << x << ", " << y << ")" << std::endl;
        }
    }
}

Wall::~Wall() {
    //if (texture) {
    //   SDL_DestroyTexture(texture);
    //}
}

void Wall::render(SDL_Renderer* renderer) {
    if (texture) {
        SDL_Rect dstRect = {x, y, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
        std::cout << "Rendering wall at (" << x << ", " << y << ") as red rect (texture failed)" << std::endl;
    }
}
