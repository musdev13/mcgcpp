#ifndef SceneManager_hpp
#define SceneManager_hpp

#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
#include "SDL2/SDL.h"
#include "VideoPlayer.hpp"

using json = nlohmann::json;

enum class SceneType {
    STATIC,
    VIDEO
};

class SceneManager {
public:
    SceneManager(SDL_Renderer* renderer);
    ~SceneManager();

    bool loadScene(const std::string& sceneName);
    void update();
    void render();
    void setGamePath(const std::string& path) { gamePath = path; }

private:
    SDL_Renderer* renderer;
    json currentScene;
    SDL_Color backgroundColor;
    std::string gamePath = ".";
    SceneType currentSceneType;
    std::string nextSceneName;
    Uint32 nextFrameTime;
    
    VideoPlayer* videoPlayer;
    
    void loadVideoScene(const json& sceneData);
    void loadStaticScene(const json& sceneData);
};

#endif
