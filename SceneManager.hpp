#ifndef SceneManager_hpp
#define SceneManager_hpp

#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
#include "SDL2/SDL.h"

using json = nlohmann::json;

class SceneManager {
public:
    SceneManager(SDL_Renderer* renderer);
    ~SceneManager();

    bool loadScene(const std::string& sceneName);
    void render();
    void setGamePath(const std::string& path) { gamePath = path; }

private:
    SDL_Renderer* renderer;
    json currentScene;
    SDL_Color backgroundColor;
    std::string gamePath = ".";  // По умолчанию текущая директория
};

#endif
