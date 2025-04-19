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
    mutable std::string displayedText; // Теперь можно изменять даже в const объектах
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
    bool isAnimating() const { return state != DialogState::Stable; }

private:
    SDL_Renderer* renderer;
    SDL_Texture* dialogBox;
    int boxHeight;
    bool active;
    float currentY;
    float targetY;
    float animationSpeed;
    
    enum class DialogState {
        Hidden,
        Opening,
        Stable,
        Closing
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

    void initializeFont(const std::string& gamePath);
    void updateAnimation(float deltaTime);
    void updateTextPrinting(float deltaTime);
    void nextLine();
    void close();
    void renderText();
};

#endif
