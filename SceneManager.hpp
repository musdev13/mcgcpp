#ifndef SceneManager_hpp
#define SceneManager_hpp

#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "VideoPlayer.hpp"

using json = nlohmann::json;

enum class SceneType {
    STATIC,
    VIDEO
};

struct Layer {
    SDL_Texture* texture;
    int zIndex;
    Uint8 opacity;
    
    Layer() : texture(nullptr), zIndex(0), opacity(255) {}
};

struct GridCell {
    SDL_Rect rect;
    int row;
    int col;
    
    std::string getId() const {
        return "r" + std::to_string(row) + "c" + std::to_string(col);
    }
};

class SceneManager {
public:
    SceneManager(SDL_Renderer* renderer);
    ~SceneManager();

    bool loadScene(const std::string& sceneName);
    void update();
    void render();
    void setGamePath(const std::string& path) { gamePath = path; }
    const GridCell* getCellAt(int row, int col) const;
    const GridCell* getCellAtPosition(int x, int y) const;
    void calculateGrid();

private:
    SDL_Renderer* renderer;
    json currentScene;
    SDL_Color backgroundColor;
    std::string gamePath = ".";
    SceneType currentSceneType;
    std::string nextSceneName;
    Uint32 nextFrameTime;
    bool showGrid;
    static const int GRID_SIZE = 48;
    
    VideoPlayer* videoPlayer;
    SDL_Texture* backgroundTexture;
    std::vector<Layer> layers;
    std::vector<std::vector<GridCell>> grid;
    int gridRows;
    int gridCols;
    
    void loadVideoScene(const json& sceneData);
    void loadStaticScene(const json& sceneData);
    void drawGrid();
    void loadBackgroundImage(const std::string& imagePath);
    void cleanupBackground();
    void loadLayers(const json& sceneData);
    void cleanupLayers();
    bool loadLayerImage(Layer& layer, const std::string& imagePath);
    void initializeGrid();
};

#endif
