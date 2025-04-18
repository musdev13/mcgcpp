#ifndef SceneManager_hpp
#define SceneManager_hpp

#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
#include "SDL2/SDL.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

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
    
    // FFmpeg components
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVFrame* frame;
    AVFrame* frameYUV;
    AVPacket* packet;
    SwsContext* swsContext;
    SDL_Texture* videoTexture;
    
    int videoStreamIndex;
    std::string nextSceneName;
    Uint32 frameDelay;
    Uint32 nextFrameTime;
    
    void loadVideoScene(const json& sceneData);
    void loadStaticScene(const json& sceneData);
    void cleanupVideo();
    bool initializeVideo(const std::string& path);
    bool decodeNextFrame();
};

#endif
