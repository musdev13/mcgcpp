#ifndef SceneManager_hpp
#define SceneManager_hpp

#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "VideoPlayer.hpp"
#include "Player.hpp"
#include "DialogSystem.hpp"
#include <variant>
#include <map>
#include <optional>

using json = nlohmann::json;

enum class SceneType {
    STATIC,
    VIDEO
};

struct Layer {
    SDL_Texture* texture;
    int zIndex;
    Uint8 opacity;
    int width;   // добавляем поле для хранения ширины
    int height;  // добавляем поле для хранения высоты
    
    Layer() : texture(nullptr), zIndex(0), opacity(255), width(0), height(0) {}
};

struct GridCell {
    SDL_Rect rect;
    int row;
    int col;
    
    std::string getId() const {
        return "r" + std::to_string(row) + "c" + std::to_string(col);
    }
};

struct ScriptCommand {
    std::string command;
    json parameter;
    float time_left = 0.0f;  // Добавляем время ожидания
    bool isComplete;
    bool isStarted;  // Добавляем флаг для отслеживания начала выполнения команды
    
    ScriptCommand() : isComplete(false), isStarted(false) {}
};

struct ScriptGroup {
    std::string name;
    std::vector<ScriptCommand> commands;
};

struct ScriptCellGroup {
    std::vector<std::pair<int, int>> cells;
    bool needUseKey;
    std::string scriptGroupName;
};

struct InitialScriptData {
    std::vector<ScriptCommand> commands;
};

class SceneManager {
public:
    SceneManager(SDL_Renderer* renderer);
    ~SceneManager();

    bool loadScene(const std::string& sceneName);
    void update();
    void render();
    void setGamePath(const std::string& path);  // Убираем inline реализацию
    const GridCell* getCellAt(int row, int col) const;
    const GridCell* getCellAtPosition(int x, int y) const;
    void calculateGrid();
    void updatePlayerVelocity(float dx, float dy);
    void update(float deltaTime);
    bool isCollision(float x, float y) const;
    std::pair<int, int> getCurrentPlayerCell() const;
    void useCurrentCell();
    void toggleDialog(const std::string& dialogName);
    void handleUseKey();
    void debugPrintVariables() const;

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
    Player player;
    std::vector<std::pair<int, int>> collisionCells;
    std::vector<ScriptCellGroup> scriptCells;
    std::vector<ScriptGroup> scriptGroups;
    std::vector<DialogGroup> dialogGroups;
    DialogSystem* dialogSystem;
    std::vector<ScriptCommand> activeCommands; // Очередь активных команд
    InitialScriptData initialScript;
    
    // Добавляем поля для эффекта fade
    float fadeAlpha = 0.0f;
    float fadeTarget = 0.0f;
    float fadeSpeed = 0.0f;
    bool isFading = false;

    // Используем variant для хранения разных типов
    using VarValue = std::variant<bool, int, float, std::string>;
    std::map<std::string, VarValue> globalVars;
    
    void loadGlobalVars(const json& sceneData);
    bool evaluateCondition(const std::string& condition);
    std::string parseVarString(const std::string& str);
    VarValue parseJsonValue(const json& value);

    void loadVideoScene(const json& sceneData);
    void loadStaticScene(const json& sceneData);
    void drawGrid();
    void loadBackgroundImage(const std::string& imagePath);
    void cleanupBackground();
    void loadLayers(const json& sceneData);
    void cleanupLayers();
    bool loadLayerImage(Layer& layer, const std::string& imagePath);
    void initializeGrid();
    void initializePlayer(const json& sceneData);
    void updatePlayerPosition();
    void renderPlayer();
    void loadCollisions(const json& sceneData);
    void loadScriptCells(const json& sceneData);
    void checkPlayerInScriptCells() const;
    void loadScriptGroups(const json& sceneData);
    void executeScriptGroup(const std::string& groupName);
    void executeCommand(const ScriptCommand& command);
    void loadDialogGroups(const json& sceneData);
    void updateActiveCommands(float deltaTime);  // Обновляем сигнатуру метода
    bool isCommandComplete(const ScriptCommand& command) const;
    void startCommand(ScriptCommand& command);
    void loadInitialScript(const json& sceneData);
    void renderFadeEffect();
    bool updateFade(float deltaTime);

    std::string trim(const std::string& str);
    bool evaluateSimpleCondition(const std::string& condition);
    std::pair<std::string, std::string> splitByOperator(const std::string& condition, const std::string& op);
    bool compareValues(const std::string& left, const std::string& right, const std::string& op);
    std::optional<float> convertToValue(const std::string& str);

    void processScriptCommands(const std::vector<ScriptCommand>& commands);
    void processCommand(const json& cmd);

    bool isExecutingScript = false;
};

#endif
