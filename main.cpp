#include "Game.hpp"
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Game *game = nullptr;

int main(int argc, char* argv[]) {
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

    // Read window settings from config
    std::string settingsPath = gamePath + "/settings.json";
    std::ifstream settingsFile(settingsPath);
    
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    std::string title = "Game";

    if (settingsFile.is_open()) {
        try {
            json settings;
            settingsFile >> settings;
            
            if (settings.contains("window")) {
                const auto& window = settings["window"];
                width = window.value("width", 800);
                height = window.value("height", 600);
                fullscreen = window.value("fullscreen", false);
                title = window.value("title", "Game");
            }
        } catch (const std::exception& e) {
            std::cout << "Error parsing settings.json: " << e.what() << std::endl;
        }
    }

    game->init(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
               width, height, fullscreen, gamePath);

    const int FPS = 40;  // Changed from 30 to 40
    const int frameDelay = 1000 / FPS;

    Uint32 frameStart;
    int frameTime;

    while (game->running()){
        frameStart = SDL_GetTicks();

        game->handleEvents();
        game->update();
        game->render();

        frameTime = SDL_GetTicks() - frameStart;
        if(frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    game->clean();
    
    return 0;
}