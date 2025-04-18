#include "SceneManager.hpp"
#include <iostream>

SceneManager::SceneManager(SDL_Renderer* renderer) : renderer(renderer) {
    backgroundColor = {255, 255, 255, 255};
    videoPlayer = new VideoPlayer(renderer);
    currentSceneType = SceneType::STATIC;
    backgroundTexture = nullptr;
}

SceneManager::~SceneManager() {
    delete videoPlayer;
    cleanupBackground();
    cleanupLayers();
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
    loadLayers(sceneData);
}

void SceneManager::loadBackgroundImage(const std::string& imagePath) {
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (surface) {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!backgroundTexture) {
            std::cout << "Failed to create texture from image: " << SDL_GetError() << std::endl;
        }
    } else {
        std::cout << "Failed to load image: " << IMG_GetError() << std::endl;
    }
}

void SceneManager::cleanupBackground() {
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
}

void SceneManager::cleanupLayers() {
    for(auto& layer : layers) {
        if(layer.texture) {
            SDL_DestroyTexture(layer.texture);
        }
    }
    layers.clear();
}

void SceneManager::loadLayers(const json& sceneData) {
    cleanupLayers();
    
    if (!sceneData.contains("layers")) return;
    
    for (const auto& layerData : sceneData["layers"]) {
        Layer layer;
        layer.zIndex = layerData.value("z", 0);
        layer.opacity = layerData.value("opacity", 255);
        
        std::string imagePath = gamePath + "/image/" + layerData["image"].get<std::string>();
        if (loadLayerImage(layer, imagePath)) {
            layers.push_back(layer);
        }
    }
    
    // Sort layers by z-index
    std::sort(layers.begin(), layers.end(), 
        [](const Layer& a, const Layer& b) { return a.zIndex < b.zIndex; });
}

bool SceneManager::loadLayerImage(Layer& layer, const std::string& imagePath) {
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (surface) {
        layer.texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!layer.texture) {
            std::cout << "Failed to create texture from image: " << SDL_GetError() << std::endl;
            return false;
        }
        return true;
    }
    std::cout << "Failed to load image: " << IMG_GetError() << std::endl;
    return false;
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
        
        // Render all layers in order
        for(const auto& layer : layers) {
            SDL_SetTextureAlphaMod(layer.texture, layer.opacity);
            SDL_RenderCopy(renderer, layer.texture, nullptr, nullptr);
        }
        
        if(showGrid && currentSceneType == SceneType::STATIC) {
            drawGrid();
        }
    }
}
