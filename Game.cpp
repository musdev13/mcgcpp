#include "Game.hpp"
#include <fstream>

Game::Game(){
    sceneManager = nullptr;
}

Game::~Game(){
    if(sceneManager) delete sceneManager;
}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen, const std::string& gamePath){
    int flags = 0;
    if (fullscreen){
        flags = SDL_WINDOW_FULLSCREEN;
    }
    
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0){
        std::cout << "Subsystems Initialized!..." << std::endl;
        
        // Initialize SDL_Image for PNG support
        if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            isRunning = false;
            return;
        }

        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if(window){
            std::cout << "Window created!" << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if(renderer){
            SDL_SetRenderDrawColor(renderer, 255,255,255,255);
            std::cout << "renderer created!" << std::endl;
            
            sceneManager = new SceneManager(renderer);
            sceneManager->setGamePath(gamePath);

            // Читаем начальную сцену из settings.json
            std::string settingsPath = gamePath + "/settings.json";
            std::ifstream settingsFile(settingsPath);
            if (settingsFile.is_open()) {
                json settings;
                settingsFile >> settings;
                sceneManager->loadScene(settings["initialScene"]);
            } else {
                sceneManager->loadScene("error"); // Fallback если файл не найден
            }
        }

        isRunning = true;
    } else{
        isRunning = false;
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_p) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if(sceneManager) {
                        const GridCell* cell = sceneManager->getCellAtPosition(mouseX, mouseY);
                        if(cell) {
                            std::cout << "Cell ID: row=" << cell->row << ", col=" << cell->col << std::endl;
                        }
                    }
                } else if(sceneManager) {
                    sceneManager->handlePlayerMovement(event.key.keysym.sym);
                }
                break;
        }
    }
}

void Game::update(){
    if(sceneManager) {
        handleMovementKeys();
        sceneManager->update();
    }
}

void Game::handleMovementKeys() {
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    
    if(!sceneManager) return;
    
    if(keyState[SDL_SCANCODE_UP]) {
        sceneManager->handlePlayerMovement(SDLK_UP);
    }
    else if(keyState[SDL_SCANCODE_DOWN]) {
        sceneManager->handlePlayerMovement(SDLK_DOWN);
    }
    else if(keyState[SDL_SCANCODE_LEFT]) {
        sceneManager->handlePlayerMovement(SDLK_LEFT);
    }
    else if(keyState[SDL_SCANCODE_RIGHT]) {
        sceneManager->handlePlayerMovement(SDLK_RIGHT);
    }
}

void Game::render(){
    SDL_RenderClear(renderer);
    
    // Рендерим текущую сцену
    if(sceneManager) {
        sceneManager->render();
    }
    
    SDL_RenderPresent(renderer);

}
void Game::clean(){
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game Cleaned!" << std::endl;
}

