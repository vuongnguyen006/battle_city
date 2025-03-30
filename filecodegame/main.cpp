#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include "Wall.h"
#include "Constants.h"
#include "Bullet.h"
#include "EnemyTanks.h"
#include "Tank.h"
#include "Game.h"

int main(int argc, char* argv[]) {
    srand(time(0));
    Game game;
    if (game.running) {
        game.run();
    }
    return 0;
}
