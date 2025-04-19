#include "Player.hpp"
#include "SDL2/SDL_image.h"
#include "SceneManager.hpp"  
#include <iostream>  // Add this line

Player::Player() : 
    x(0),
    y(0),
    velocityX(0),
    velocityY(0),
    speed(100.0f), // Reduced from 200.0f to 100.0f pixels per second
    color{0, 0, 255, 255},
    size(0),
    spriteSheet(nullptr),
    frameWidth(48),  // Changed from 32 to 48
    frameHeight(48), // Changed from 32 to 48
    currentFrame(0),
    frameTimer(0),
    frameDuration(0.1f), // 10 frames per second for animation
    currentState("idle"),
    currentDirection(Direction::DOWN),
    lastDirection(Direction::DOWN),
    scale(2.0f)  // Make player 2x bigger
{
    rect = {0, 0, 0, 0};
    srcRect = {0, 0, frameWidth, frameHeight};
}

Player::~Player() {
    if(spriteSheet) {
        SDL_DestroyTexture(spriteSheet);
    }
}

bool Player::loadSprite(SDL_Renderer* renderer, const std::string& path) {
    if(spriteSheet) {
        SDL_DestroyTexture(spriteSheet);
        spriteSheet = nullptr;
    }
    
    SDL_Surface* surface = IMG_Load(path.c_str());
    if(!surface) {
        SDL_Log("Failed to load sprite sheet: %s", IMG_GetError());
        return false;
    }
    
    spriteSheet = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if(!spriteSheet) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return false;
    }
    
    return true;
}

void Player::setPosition(float newX, float newY, int newSize) {
    x = newX;
    y = newY;
    size = newSize;
    
    // Adjust position and size for scale
    rect.w = size * scale;
    rect.h = size * scale;
    rect.x = (int)(x - (rect.w - size) / 2); // Center the scaled sprite
    rect.y = (int)(y - (rect.h - size) / 2);
}

void Player::setAnimation(const std::string& state) {
    // Remove unused variable stateChanged
    currentState = state;
    
    // Initialize row with a default value
    int row = 0;
    
    if(state == "walk") {
        switch(currentDirection) {
            case Direction::DOWN:  row = 0; break;
            case Direction::LEFT:  row = 1; break;
            case Direction::UP:    row = 2; break;
            case Direction::RIGHT: row = 3; break;
        }
    } else { // idle
        switch(lastDirection) {
            case Direction::DOWN:  row = 0; break;
            case Direction::LEFT:  row = 1; break;
            case Direction::UP:    row = 2; break;
            case Direction::RIGHT: row = 3; break;
        }
        currentFrame = 0; // Reset to first frame only for idle
    }
    srcRect.y = row * frameHeight;
}

void Player::setDirection(float dx, float dy) {
    Direction newDirection = currentDirection;
    
    if(std::abs(dx) > std::abs(dy)) {
        if(dx < 0) newDirection = Direction::LEFT;
        else if(dx > 0) newDirection = Direction::RIGHT;
    } else if(std::abs(dy) > std::abs(dx)) {
        if(dy < 0) newDirection = Direction::UP;
        else if(dy > 0) newDirection = Direction::DOWN;
    }
    
    // Fix operator precedence with parentheses
    if((dx != 0 || dy != 0) && newDirection != currentDirection) {
        currentDirection = newDirection;
        lastDirection = newDirection;
        setAnimation("walk");
    }
}

void Player::update(float deltaTime) {
    if (!movementEnabled) {
        velocityX = 0;
        velocityY = 0;
        updateAnimation(deltaTime);
        return;
    }

    float newX = x + velocityX * speed * deltaTime;
    float newY = y;
    
    // Get player collision points
    float centerY = y + size + size/2 - 4;
    float leftX = newX + size/4;
    float centerX = newX + size/2;
    float rightX = newX + size*3/4;
    
    // Проверяем коллизию по X
    if(!checkCollision(leftX, centerY) && 
       !checkCollision(centerX, centerY) && 
       !checkCollision(rightX, centerY)) {
        x = newX;
    }

    // Проверяем коллизию по Y
    newX = x;
    newY = y + velocityY * speed * deltaTime;
    
    // Обновляем Y-координаты для точек
    centerY = newY + size + size/2 - 4;
    leftX = x + size/4;
    centerX = x + size/2;
    rightX = x + size*3/4;
    
    if(!checkCollision(leftX, centerY) && 
       !checkCollision(centerX, centerY) && 
       !checkCollision(rightX, centerY)) {
        y = newY;
    }
    
    rect.x = (int)(x - (rect.w - size) / 2);
    rect.y = (int)(y - (rect.h - size) / 2);
    
    // Update direction
    setDirection(velocityX, velocityY);
    
    // Update animation
    if(velocityX != 0 || velocityY != 0) {
        if(currentState != "walk") {
            setAnimation("walk");
        }
    } else if(currentState != "idle") {
        setAnimation("idle");
    }
    
    updateAnimation(deltaTime);
}

bool Player::checkCollision(float newX, float newY) const {
    if(!sceneManager) return false;
    bool collision = sceneManager->isCollision(newX, newY);
    if(collision) {
        std::cout << "Collision at pixel: (" << newX << ", " << newY << ")" << std::endl;
    }
    return collision;
}

void Player::updateAnimation(float deltaTime) {
    if(!spriteSheet) return;
    
    frameTimer += deltaTime;
    if(frameTimer >= frameDuration) {
        frameTimer = 0;
        if(currentState == "walk") {
            currentFrame = (currentFrame + 1) % 3; // 3 frames per animation
        }
        srcRect.x = currentFrame * frameWidth;
    }
}

void Player::render(SDL_Renderer* renderer) const {
    if(spriteSheet) {
        SDL_RenderCopy(renderer, spriteSheet, &srcRect, &rect);
        
        // Отрисовка точек коллизии (для отладки)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        
        float centerY = y + size + size/2 - 4;
        float leftX = x + size/4;
        float centerX = x + size/2;
        float rightX = x + size*3/4;
        
        // Рисуем линию между точками
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, 
            static_cast<int>(leftX), 
            static_cast<int>(centerY),
            static_cast<int>(rightX), 
            static_cast<int>(centerY)
        );
        
        // Рисуем точки
        SDL_Rect point;
        point.w = 4;
        point.h = 4;
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Красные точки
        
        point.x = static_cast<int>(leftX) - 2;
        point.y = static_cast<int>(centerY) - 2;
        SDL_RenderFillRect(renderer, &point);
        
        point.x = static_cast<int>(centerX) - 2;
        SDL_RenderFillRect(renderer, &point);
        
        point.x = static_cast<int>(rightX) - 2;
        SDL_RenderFillRect(renderer, &point);
    } else {
        // Fallback to rectangle rendering
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}
