#include "SceneManager.hpp"
#include <iostream>

SceneManager::SceneManager(SDL_Renderer* renderer) : renderer(renderer) {
    backgroundColor = {255, 255, 255, 255};
    videoPlayer = new VideoPlayer(renderer);
    currentSceneType = SceneType::STATIC;
}

SceneManager::~SceneManager() {
    delete videoPlayer;
}

bool SceneManager::loadScene(const std::string& sceneName) {
    std::string filePath = gamePath + "/scenes/" + sceneName + ".json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cout << "Failed to open scene file: " << filePath << std::endl;
        return false;
    }

    try {
        file >> currentScene;
        
        std::string sceneType = currentScene["type"];
        if(sceneType == "video") {
            currentSceneType = SceneType::VIDEO;
            loadVideoScene(currentScene);
        } else {
            currentSceneType = SceneType::STATIC;
            loadStaticScene(currentScene);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error parsing scene file: " << e.what() << std::endl;
        return false;
    }
}

void SceneManager::loadVideoScene(const json& sceneData) {
    std::string videoPath = gamePath + "/video/" + sceneData["videoFile"].get<std::string>();
    nextSceneName = sceneData["nextScene"];
    
    if(!videoPlayer->initialize(videoPath)) {
        std::cout << "Failed to initialize video: " << videoPath << std::endl;
        loadScene(nextSceneName);
    }
    nextFrameTime = SDL_GetTicks();
}

void SceneManager::loadStaticScene(const json& sceneData) {
    auto bgColor = sceneData["backgroundColor"];
    backgroundColor.r = bgColor["r"];
    backgroundColor.g = bgColor["g"];
    backgroundColor.b = bgColor["b"];
    backgroundColor.a = bgColor["a"];
    
    showGrid = sceneData.value("showGrid", false);
}

void SceneManager::drawGrid() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    
    // Вертикальные линии
    for(int x = 0; x <= w; x += GRID_SIZE) {
        SDL_RenderDrawLine(renderer, x, 0, x, h);
    }
    
    // Горизонтальные линии
    for(int y = 0; y <= h; y += GRID_SIZE) {
        SDL_RenderDrawLine(renderer, 0, y, w, y);
    }
}

void SceneManager::update() {
    if(currentSceneType == SceneType::VIDEO) {
        Uint32 currentTime = SDL_GetTicks();
        if(currentTime >= nextFrameTime) {
            if(!videoPlayer->decodeNextFrame()) {
                loadScene(nextSceneName);
                return;
            }
            nextFrameTime = currentTime + videoPlayer->getFrameDelay();
            
            if (currentTime > nextFrameTime + videoPlayer->getFrameDelay() * 2) {
                nextFrameTime = currentTime;
            }
        }
    }
}

void SceneManager::render() {
    if(currentSceneType == SceneType::VIDEO && videoPlayer->getTexture()) {
        SDL_RenderCopy(renderer, videoPlayer->getTexture(), nullptr, nullptr);
    } else {
        SDL_SetRenderDrawColor(renderer, 
            backgroundColor.r, 
            backgroundColor.g, 
            backgroundColor.b, 
            backgroundColor.a);
        SDL_RenderClear(renderer);
        
        if(showGrid && currentSceneType == SceneType::STATIC) {
            drawGrid();
        }
    }
}
