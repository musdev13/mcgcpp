#include "Game.hpp"
#include <string>

Game *game = nullptr;

int main(int argc, char* argv[]){
    std::string gamePath = ".";

    // Обработка аргументов командной строки
    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if(arg == "--game-path" && i + 1 < argc) {
            gamePath = argv[i + 1];
            i++; // Пропускаем следующий аргумент, так как мы его уже обработали
        }
    }

    game = new Game();
    game->init("title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, false, gamePath);

    while (game->running()){
        game->handleEvents();
        game->update();
        game->render();
    }

    game->clean();
    
    return 0;
}