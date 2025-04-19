#include "DialogSystem.hpp"
#include <iostream>

DialogSystem::DialogSystem(SDL_Renderer* renderer, const std::string& gamePath) 
    : renderer(renderer),
      dialogBox(nullptr),
      boxHeight(250),
      active(false), 
      currentY(600),
      targetY(600),
      animationSpeed(1000.0f), 
      state(DialogState::Hidden),
      currentGroup(nullptr),
      currentLineIndex(0),
      textTimer(0),
      charDelay(0.05f),
      instantPrint(false),
      font(nullptr),
      fontSize(36),  // Изменили с 32 на 36 для основного текста
      textColor{255, 255, 255, 255},
      titleColor{255, 200, 0, 255},
      textPadding(30)  // Увеличили с 20 до 30
{
    if(TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    loadDialogBox(gamePath + "/image/dialogBox.png");
    initializeFont(gamePath);
}

DialogSystem::~DialogSystem() {
    if(dialogBox) {
        SDL_DestroyTexture(dialogBox);
    }
    if(font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

void DialogSystem::loadDialogBox(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if(surface) {
        dialogBox = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
}

void DialogSystem::initializeFont(const std::string& gamePath) {
    std::string fontPath = gamePath + "/font/Mcg.ttf"; // Изменено на Mcg.ttf
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if(!font) {
        std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
}

void DialogSystem::render() {
    if(!active) return;

    // Рендерим диалоговое окно
    SDL_Rect dstRect = {0, static_cast<int>(currentY), 800, boxHeight};
    SDL_RenderCopy(renderer, dialogBox, nullptr, &dstRect);

    // Рендерим текст
    if(currentGroup && currentLineIndex < currentGroup->lines.size()) {
        renderText();
    }
}

void DialogSystem::renderText() {
    if(!font || !currentGroup || currentLineIndex >= currentGroup->lines.size()) return;

    const auto& line = currentGroup->lines[currentLineIndex];
    
    // Устанавливаем размер шрифта для заголовка на 12 пикселей больше основного
    TTF_SetFontSize(font, fontSize + 12);
    
    // Рендерим заголовок
    SDL_Surface* titleSurface = TTF_RenderText_Blended(font, line.title.c_str(), titleColor);
    if(titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_Rect titleRect = {
            textPadding, 
            static_cast<int>(currentY) + textPadding,
            titleSurface->w,
            titleSurface->h
        };
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
        SDL_FreeSurface(titleSurface);
        SDL_DestroyTexture(titleTexture);
    }

    // Возвращаем размер шрифта для основного текста
    TTF_SetFontSize(font, fontSize);
    
    // Рендерим текст
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, line.displayedText.c_str(), textColor);
    if(textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            textPadding,
            static_cast<int>(currentY) + textPadding + fontSize + 15, // Увеличили отступ между заголовком и текстом с 10 до 15
            textSurface->w,
            textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

void DialogSystem::update(float deltaTime) {
    if(!active) return;

    updateAnimation(deltaTime);
    if(state == DialogState::Stable) {
        updateTextPrinting(deltaTime);
    }
}

void DialogSystem::updateAnimation(float deltaTime) {
    if(state == DialogState::Opening) {
        currentY -= animationSpeed * deltaTime;
        if(currentY <= targetY) {
            currentY = targetY;
            state = DialogState::Stable;
        }
    }
    else if(state == DialogState::Closing) {
        currentY += animationSpeed * deltaTime;
        if(currentY >= 600) {
            currentY = 600;
            state = DialogState::Hidden;
            active = false;
            currentLineIndex = 0;
            currentGroup = nullptr;
        }
    }
}

void DialogSystem::updateTextPrinting(float deltaTime) {
    if(!currentGroup || currentLineIndex >= currentGroup->lines.size()) return;

    auto& line = currentGroup->lines[currentLineIndex];
    if(instantPrint) {
        line.displayedText = line.text;
        instantPrint = false;
        return;
    }

    if(line.displayedText.length() < line.text.length()) {
        textTimer += deltaTime;
        if(textTimer >= charDelay) {
            textTimer = 0;
            line.displayedText += line.text[line.displayedText.length()];
        }
    }
}

void DialogSystem::handleInput(bool useKeyPressed) {
    if(!active || state != DialogState::Stable) return;

    const auto& line = currentGroup->lines[currentLineIndex];
    if(line.displayedText.length() < line.text.length()) {
        instantPrint = true;
    } else {
        nextLine();
    }
}

void DialogSystem::nextLine() {
    if(!currentGroup) return;
    
    currentLineIndex++;
    if(currentLineIndex >= currentGroup->lines.size()) {
        close();
    } else {
        currentGroup->lines[currentLineIndex].displayedText = "";
        textTimer = 0;
    }
}

void DialogSystem::showDialog(const std::string& dialogGroupName) {
    if(state != DialogState::Hidden) return;
    
    active = true;
    state = DialogState::Opening;
    targetY = 600 - boxHeight;
    currentY = 600;
}

void DialogSystem::setCurrentGroup(const DialogGroup* group) {
    currentGroup = const_cast<DialogGroup*>(group); // Преобразуем const в non-const
    currentLineIndex = 0;
    if(group && !group->lines.empty()) {
        group->lines[0].displayedText = "";
    }
}

void DialogSystem::close() {
    if(state == DialogState::Stable) {
        state = DialogState::Closing;
        targetY = 600;
    }
}
