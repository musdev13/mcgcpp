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
                }
                break;
        }
    }
}

void Game::update() {
    if(sceneManager) {
        handleMovementKeys();
        sceneManager->update(1.0f / 30.0f); // Pass fixed deltaTime for 30 FPS
    }
}

void Game::handleMovementKeys() {
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    
    if(!sceneManager) return;
    
    float dx = 0.0f, dy = 0.0f;
    
    if(keyState[SDL_SCANCODE_UP]) {
        dy = -1.0f;
    }
    if(keyState[SDL_SCANCODE_DOWN]) {
        dy = 1.0f;
    }
    if(keyState[SDL_SCANCODE_LEFT]) {
        dx = -1.0f;
    }
    if(keyState[SDL_SCANCODE_RIGHT]) {
        dx = 1.0f;
    }
    
    // Normalize diagonal movement
    if (dx != 0.0f && dy != 0.0f) {
        const float normalize = 0.707107f; // 1/√2
        dx *= normalize;
        dy *= normalize;
    }
    
    sceneManager->updatePlayerVelocity(dx, dy);
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

