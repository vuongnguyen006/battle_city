#ifndef GAME_H
#define GAME_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <string>
#include "Wall.h"
#include "Bullet.h"
#include "Tank.h"
#include "EnemyTanks.h"
#include "Constants.h"

class Game {
public:
    enum GameState { MENU, PLAYING, PAUSED};
    GameState gameState;
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    std::vector<Wall> walls;
    std::vector<EnemyTank> enemies;
    Tank player1;
    TTF_Font* font;
    int lives;
    int score;
    int highScore;
    int selectedMenuItem;
    SDL_Texture* tanks;
    SDL_Texture* eagleTexture;
    SDL_Rect protectedZone;
    bool protectedZoneActive;
    Mix_Chunk* wallBreakSound;
    Mix_Music* menuSound;

    Game();
    ~Game();
    void run();

private:
    char map[MAP_HEIGHT][MAP_WIDTH];
    std::vector<int> spawnX;
    std::vector<int> spawnY;
    void loadMap();
    void generateWalls();
    void spawnEnemies();
    void resetGame();
    void handleEvents();
    void update();
    void render();
};

#endif
