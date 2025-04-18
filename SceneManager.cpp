#include "SceneManager.hpp"
#include <iostream>

SceneManager::SceneManager(SDL_Renderer* renderer) : renderer(renderer) {
    backgroundColor = {255, 255, 255, 255};
}

SceneManager::~SceneManager() {}

bool SceneManager::loadScene(const std::string& sceneName) {
    std::string filePath = gamePath + "/scenes/" + sceneName + ".json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cout << "Failed to open scene file: " << filePath << std::endl;
        return false;
    }

    try {
        file >> currentScene;
        
        // Загружаем цвет фона из JSON
        auto bgColor = currentScene["backgroundColor"];
        backgroundColor.r = bgColor["r"];
        backgroundColor.g = bgColor["g"];
        backgroundColor.b = bgColor["b"];
        backgroundColor.a = bgColor["a"];
        
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error parsing scene file: " << e.what() << std::endl;
        return false;
    }
}

void SceneManager::render() {
    SDL_SetRenderDrawColor(renderer, 
        backgroundColor.r, 
        backgroundColor.g, 
        backgroundColor.b, 
        backgroundColor.a);
}
