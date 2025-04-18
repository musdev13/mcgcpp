#ifndef Game_hpp
#define Game_hpp

#include "SDL2/SDL.h"
#include "SceneManager.hpp"
#include <iostream>

class Game {
public:
    Game();
    ~Game();

    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen, const std::string& gamePath);
    
    void handleEvents();
    void update();
    void render();
    void clean();

    bool running(){ return isRunning; }
private:
    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SceneManager* sceneManager;

    void handleMovementKeys(); // Новый метод для обработки клавиш движения
};

#endif