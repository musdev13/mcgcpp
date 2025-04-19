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
    // Проверяем коллизии отдельно для X и Y направлений
    float newX = x + velocityX * speed * deltaTime;
    float newY = y;
    
    // Проверяем коллизию по X
    if(!checkCollision(newX + size/2, newY + size)) {
        x = newX;
    }

    // Проверяем коллизию по Y
    newX = x;
    newY = y + velocityY * speed * deltaTime;
    
    if(!checkCollision(newX + size/2, newY + size)) {
        y = newY;
    }
    
    rect.x = (int)(x - (rect.w - size) / 2);
    rect.y = (int)(y - (rect.h - size) / 2);
    
    // Update direction
    setDirection(velocityX, velocityY);
    
    // Update animation based on movement
    if(velocityX != 0 || velocityY != 0) {
        if(currentState != "walk") {
            setAnimation("walk");
        }
    } else if(currentState != "idle") {
        setAnimation("idle");
    }
    
    // Always update animation frames
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
    } else {
        // Fallback to rectangle rendering
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}
