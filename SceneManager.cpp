#include "SceneManager.hpp"
#include <iostream>

SceneManager::SceneManager(SDL_Renderer* renderer) : renderer(renderer) {
    backgroundColor = {255, 255, 255, 255};
    videoPlayer = new VideoPlayer(renderer);
    currentSceneType = SceneType::STATIC;
    backgroundTexture = nullptr;
    gridRows = 0;
    gridCols = 0;
    dialogSystem = nullptr; // Сначала nullptr
    calculateGrid();
}

SceneManager::~SceneManager() {
    delete videoPlayer;
    cleanupBackground();
    cleanupLayers();
    delete dialogSystem;
}

void SceneManager::setGamePath(const std::string& path) { 
    gamePath = path;
    // Создаем DialogSystem после установки пути
    if(!dialogSystem) {
        dialogSystem = new DialogSystem(renderer, gamePath);
    }
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
    loadCollisions(sceneData);
    loadScriptGroups(sceneData);
    loadScriptCells(sceneData);
    calculateGrid();
    initializePlayer(sceneData);
    loadDialogGroups(sceneData);
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
        // Создаем текстуру с фиксированным размером
        layer.texture = SDL_CreateTextureFromSurface(renderer, surface);
        // Сохраняем оригинальные размеры изображения
        layer.width = surface->w;
        layer.height = surface->h;
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

void SceneManager::calculateGrid() {
    if(currentSceneType != SceneType::STATIC) {
        return;
    }
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    // Пересчитываем размеры сетки под новый размер окна
    gridCols = w / GRID_SIZE;
    gridRows = h / GRID_SIZE;
    initializeGrid();
}

void SceneManager::initializeGrid() {
    grid.resize(gridRows);
    for(int row = 0; row < gridRows; row++) {
        grid[row].resize(gridCols);
        for(int col = 0; col < gridCols; col++) {
            GridCell& cell = grid[row][col];
            cell.row = row;
            cell.col = col;
            cell.rect.x = col * GRID_SIZE;
            cell.rect.y = row * GRID_SIZE;
            cell.rect.w = GRID_SIZE;
            cell.rect.h = GRID_SIZE;
        }
    }
}

const GridCell* SceneManager::getCellAt(int row, int col) const {
    // Возвращаем nullptr если сцена не статическая
    if(currentSceneType != SceneType::STATIC) {
        return nullptr;
    }
    if(row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
        return &grid[row][col];
    }
    return nullptr;
}

const GridCell* SceneManager::getCellAtPosition(int x, int y) const {
    // Возвращаем nullptr если сцена не статическая
    if(currentSceneType != SceneType::STATIC) {
        return nullptr;
    }
    int row = y / GRID_SIZE;
    int col = x / GRID_SIZE;
    return getCellAt(row, col);
}

void SceneManager::drawGrid() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    // Draw vertical lines
    for(int col = 0; col <= gridCols; col++) {
        int x = col * GRID_SIZE;
        SDL_RenderDrawLine(renderer, x, 0, x, gridRows * GRID_SIZE);
    }
    // Draw horizontal lines
    for(int row = 0; row <= gridRows; row++) {
        int y = row * GRID_SIZE;
        SDL_RenderDrawLine(renderer, 0, y, gridCols * GRID_SIZE, y);
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
    } else if(currentSceneType == SceneType::STATIC) {
        updatePlayerPosition();
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
        // Рендерим слои с фиксированным размером
        for(const auto& layer : layers) {
            SDL_SetTextureAlphaMod(layer.texture, layer.opacity);
            // Создаем прямоугольник назначения с оригинальными размерами
            SDL_Rect dstRect = {0, 0, layer.width, layer.height};
            // Центрируем изображение, если оно меньше окна
            int w, h;
            SDL_GetRendererOutputSize(renderer, &w, &h);
            dstRect.x = (w - layer.width) / 2;
            dstRect.y = (h - layer.height) / 2;
            SDL_RenderCopy(renderer, layer.texture, nullptr, &dstRect);
        }
        if(showGrid && currentSceneType == SceneType::STATIC) {
            drawGrid();
        }
        if(currentSceneType == SceneType::STATIC) {
            player.render(renderer);
            
            // Рендерим коллизии (отладочное отображение)
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); // Полупрозрачный красный
            for(const auto& cell : collisionCells) {
                SDL_Rect collisionRect = {
                    cell.second * GRID_SIZE,  // x = col * size
                    cell.first * GRID_SIZE,   // y = row * size
                    GRID_SIZE,                // width
                    GRID_SIZE                 // height
                };
                SDL_RenderDrawRect(renderer, &collisionRect);
            }
            
            // Отображаем точку коллизии игрока 
            float playerCollisionX = player.getX() + player.getSize()/2;
            float playerCollisionY = player.getY() + player.getSize() + player.getSize()/2 - 4; // Подняли на 4 пикселя
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect playerPoint = {
                static_cast<int>(playerCollisionX) - 2,
                static_cast<int>(playerCollisionY) - 2,
                4, 4
            };
            SDL_RenderFillRect(renderer, &playerPoint);
        }
        // Рендерим скрипт-клетки
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128); // Полупрозрачный синий
        for(const auto& group : scriptCells) {
            for(const auto& cell : group.cells) {
                SDL_Rect scriptRect = {
                    cell.second * GRID_SIZE,  // x = col * size
                    cell.first * GRID_SIZE,   // y = row * size
                    GRID_SIZE,                // width
                    GRID_SIZE                 // height
                };
                SDL_RenderDrawRect(renderer, &scriptRect);
            }
        }
    }
    if(dialogSystem) {
        dialogSystem->render();
    }
}

void SceneManager::initializePlayer(const json& sceneData) {
    if (!sceneData.contains("player")) return;
    
    auto playerData = sceneData["player"];
    float x = playerData.value("col", 0) * GRID_SIZE;
    float y = playerData.value("row", 0) * GRID_SIZE;
    player.setPosition(x, y, GRID_SIZE);
    
    float speed = playerData.value("speed", 100.0f);
    player.setSpeed(speed);
    
    std::string playerSkin = playerData.value("skin", "marko");
    std::string spritePath = gamePath + "/skin/" + playerSkin + "/spritesheet.png";
    if (!player.loadSprite(renderer, spritePath)) {
        std::cout << "Failed to load player skin: " << playerSkin << std::endl;
    }
    
    player.setCollisionChecker(this);
}

void SceneManager::updatePlayerVelocity(float dx, float dy) {
    player.setVelocity(dx, dy);
}

void SceneManager::update(float deltaTime) {
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
    } else if(currentSceneType == SceneType::STATIC) {
        player.update(deltaTime);
        checkPlayerInScriptCells();
        updateActiveCommands(deltaTime); // Добавляем обновление команд
    }
    if(dialogSystem) {
        dialogSystem->update(deltaTime);
    }
}

void SceneManager::updatePlayerPosition() {
    player.update(1.0f / 40.0f);
}

void SceneManager::loadCollisions(const json& sceneData) {
    collisionCells.clear();
    
    if(sceneData.contains("collisions")) {
        for(const auto& cell : sceneData["collisions"]) {
            int row = cell["row"];
            int col = cell["col"];
            collisionCells.emplace_back(row, col);
            std::cout << "Added collision at: [" << row << ", " << col << "]" << std::endl;
        }
    }
}

void SceneManager::loadScriptGroups(const json& sceneData) {
    scriptGroups.clear();
    
    if(sceneData.contains("ScriptGroups")) {
        for(const auto& groupData : sceneData["ScriptGroups"]) {
            ScriptGroup group;
            group.name = groupData["name"];
            
            for(const auto& cmdData : groupData["script"]) {
                for(auto it = cmdData.begin(); it != cmdData.end(); ++it) {
                    ScriptCommand cmd;
                    cmd.command = it.key();
                    cmd.parameter = it.value();
                    group.commands.push_back(cmd);
                }
            }
            
            scriptGroups.push_back(group);
        }
    }
}

void SceneManager::loadScriptCells(const json& sceneData) {
    scriptCells.clear();
    
    if(sceneData.contains("ScriptCells")) {
        for(const auto& group : sceneData["ScriptCells"]) {
            ScriptCellGroup cellGroup;
            cellGroup.needUseKey = group.value("needUseKey", true);
            cellGroup.scriptGroupName = group["scriptGroup"];
            
            for(const auto& cell : group["cells"]) {
                int row = cell["row"];
                int col = cell["col"];
                cellGroup.cells.emplace_back(row, col);
            }
            scriptCells.push_back(cellGroup);
        }
    }
}

void SceneManager::loadDialogGroups(const json& sceneData) {
    dialogGroups.clear();
    
    if(sceneData.contains("DialogGroups")) {
        for(const auto& groupData : sceneData["DialogGroups"]) {
            DialogGroup group;
            group.name = groupData["name"];
            
            for(const auto& lineData : groupData["content"]) {
                DialogLine line;
                line.title = lineData["title"];
                line.text = lineData["text"];
                line.avatar = lineData.value("avatar", "none");
                line.animDuration = lineData.value("animDuration", 1.0f); // Добавляем загрузку времени анимации
                line.displayedText = "";
                group.lines.push_back(line);
            }
            
            dialogGroups.push_back(group);
        }
    }
}

bool SceneManager::isCollision(float x, float y) const {
    // Преобразуем координаты в индексы сетки
    int col = static_cast<int>(std::floor(x / GRID_SIZE));
    int row = static_cast<int>(std::floor(y / GRID_SIZE));
    
    // Проверяем выход за пределы сетки
    if(row < 0 || row >= gridRows || col < 0 || col >= gridCols) {
        return true; // Считаем выход за пределы карты коллизией
    }
    
    // std::cout << "Checking collision at grid: [" << row << ", " << col << "]" << std::endl;
    
    return std::find(collisionCells.begin(), collisionCells.end(), 
                    std::make_pair(row, col)) != collisionCells.end();
}

std::pair<int, int> SceneManager::getCurrentPlayerCell() const {
    // Получаем координаты точки коллизии игрока
    float playerCollisionX = player.getX() + player.getSize()/2;
    float playerCollisionY = player.getY() + player.getSize() + player.getSize()/2 - 4;
    
    // Преобразуем координаты в индексы сетки
    int col = static_cast<int>(std::floor(playerCollisionX / GRID_SIZE));
    int row = static_cast<int>(std::floor(playerCollisionY / GRID_SIZE));
    
    return {row, col};
}

void SceneManager::checkPlayerInScriptCells() const {
    float playerLeft = player.getX() + player.getSize()/4;
    float playerRight = player.getX() + player.getSize()*3/4;
    float playerY = player.getY() + player.getSize() + player.getSize()/2 - 4;
    
    int playerRow = static_cast<int>(std::floor(playerY / GRID_SIZE));
    int leftCol = static_cast<int>(std::floor(playerLeft / GRID_SIZE));
    int rightCol = static_cast<int>(std::floor(playerRight / GRID_SIZE));
    
    for(size_t i = 0; i < scriptCells.size(); i++) {
        const auto& group = scriptCells[i];
        for(const auto& cell : group.cells) {
            if(cell.first == playerRow && 
               (cell.second == leftCol || cell.second == rightCol)) {
                if(!group.needUseKey) {
                    const_cast<SceneManager*>(this)->executeScriptGroup(group.scriptGroupName);
                }
            }
        }
    }
}

void SceneManager::useCurrentCell() {
    if(currentSceneType != SceneType::STATIC) return;
    
    float playerLeft = player.getX() + player.getSize()/4;
    float playerRight = player.getX() + player.getSize()*3/4;
    float playerY = player.getY() + player.getSize() + player.getSize()/2 - 4;
    
    int playerRow = static_cast<int>(std::floor(playerY / GRID_SIZE));
    int leftCol = static_cast<int>(std::floor(playerLeft / GRID_SIZE));
    int rightCol = static_cast<int>(std::floor(playerRight / GRID_SIZE));
    
    for(size_t i = 0; i < scriptCells.size(); i++) {
        const auto& group = scriptCells[i];
        if(group.needUseKey) {
            for(const auto& cell : group.cells) {
                if(cell.first == playerRow && 
                   (cell.second == leftCol || cell.second == rightCol)) {
                    executeScriptGroup(group.scriptGroupName);
                }
            }
        }
    }
}

void SceneManager::executeScriptGroup(const std::string& groupName) {
    activeCommands.clear(); // Очищаем предыдущие команды
    
    // Находим группу и добавляем все её команды в очередь
    for(const auto& group : scriptGroups) {
        if(group.name == groupName) {
            activeCommands = group.commands;
            break;
        }
    }
}

void SceneManager::updateActiveCommands(float deltaTime) {
    if (activeCommands.empty()) return;

    auto& currentCommand = activeCommands.front();
    
    if (!currentCommand.isStarted) {
        startCommand(currentCommand);
        currentCommand.isStarted = true;
    }
    
    if (currentCommand.command == "wait") {
        currentCommand.time_left -= deltaTime;
        if (currentCommand.time_left <= 0) {
            currentCommand.isComplete = true;
        }
    }
    
    if (isCommandComplete(currentCommand)) {
        activeCommands.erase(activeCommands.begin());
    }
}

void SceneManager::startCommand(ScriptCommand& command) {
    if (command.command == "showDialog") {
        toggleDialog(command.parameter);
    } else if (command.command == "showDebugMessage") {
        std::cout << "Debug: " << command.parameter << std::endl;
        command.isComplete = true;
    } else if (command.command == "wait") {
        command.time_left = static_cast<float>(command.parameter.get<double>());
    }
}

bool SceneManager::isCommandComplete(const ScriptCommand& command) const {
    if (command.command == "showDialog") {
        return !dialogSystem->isActive();
    } else if (command.command == "showDebugMessage") {
        return command.isComplete;
    } else if (command.command == "wait") {
        return command.isComplete;
    }
    return true;
}

void SceneManager::executeCommand(const ScriptCommand& command) {
    // Этот метод теперь только добавляет команды в очередь
    activeCommands.push_back(command);
}

void SceneManager::toggleDialog(const std::string& dialogName) {
    if(!dialogSystem->isActive()) {
        for(const auto& group : dialogGroups) {
            if(group.name == dialogName) {
                dialogSystem->setCurrentGroup(&group);
                dialogSystem->showDialog(dialogName);
                break;
            }
        }
    }
}

void SceneManager::handleUseKey() {
    if(dialogSystem && dialogSystem->isActive()) {
        dialogSystem->handleInput(true); // Этот вызов должен работать
    } else {
        useCurrentCell();
    }
}
