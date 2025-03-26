
#ifndef GAME_H
#define GAME_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include "Wall.h"
#include "Bullet.h"
#include "Tank.h"
#include "EnemyTanks.h"
#include "Constants.h"

class Game {
public:
    enum GameState { MENU, PLAYING };
    GameState gameState;
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    std::vector<Wall> walls;
    std::vector<EnemyTank> enemies;
    Tank player1;
    TTF_Font* font;
    int score;
    int highScore;
    int selectedMenuItem;
    SDL_Texture* tanks;
    SDL_Rect protectedZone;
    bool protectedZoneActive;

    Game();
    ~Game();
    void run();

private:
    void generateWalls();
    void spawnEnemies();
    void resetGame();
    void handleEvents();
    void update();
    void render();
};

#endif
