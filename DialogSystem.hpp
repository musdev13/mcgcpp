#ifndef DialogSystem_hpp
#define DialogSystem_hpp

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <string>
#include <vector>

struct DialogLine {
    std::string title;
    std::string text;
    std::string avatar;  // Путь к аватару или "none"
    float animDuration;  // Добавляем время анимации в секундах
    mutable std::string displayedText;
    mutable SDL_Texture* avatarTexture;  // Кэшированная текстура аватара
    
    DialogLine() : avatarTexture(nullptr), animDuration(1.0f) {}
};

struct DialogGroup {
    std::string name;
    std::vector<DialogLine> lines;
};

class DialogSystem {
public:
    DialogSystem(SDL_Renderer* renderer, const std::string& gamePath);
    ~DialogSystem();
    void loadDialogBox(const std::string& path);
    void render();
    void update(float deltaTime);
    void handleInput(bool useKeyPressed);
    void showDialog(const std::string& dialogGroupName);
    void setCurrentGroup(const DialogGroup* group);
    bool isActive() const { return active; }
    bool isAnimating() const { return state == DialogState::Opening || state == DialogState::Closing; }
    bool wasJustClosed() const { return state == DialogState::Hidden && !active; }

private:
    SDL_Renderer* renderer;
    SDL_Texture* dialogBox;
    int boxHeight;
    bool active;
    float currentY;
    float targetY;
    float animationSpeed;
    std::string gamePath;  // Добавляем путь к игре
    
    enum class DialogState {
        Hidden,
        Opening,
        Stable,
        Closing,
    } state;

    DialogGroup* currentGroup; // Убираем const
    size_t currentLineIndex;
    float textTimer;
    float charDelay;
    bool instantPrint;
    TTF_Font* font;
    int fontSize;
    SDL_Color textColor;
    SDL_Color titleColor;
    int textPadding;
    static const int AVATAR_SIZE = 212;

    void initializeFont(const std::string& gamePath);
    void updateAnimation(float deltaTime);
    void updateTextPrinting(float deltaTime);
    void nextLine();
    void close();
    void renderText();
    void loadAvatar(DialogLine& line, const std::string& gamePath);
    void cleanupAvatars();
};

#endif
