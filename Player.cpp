#include "Player.hpp"

Player::Player() : 
    x(0),
    y(0),
    velocityX(0),
    velocityY(0),
    speed(200.0f), // pixels per second
    color{0, 0, 255, 255},
    size(0)
{
    rect = {0, 0, 0, 0};
}

void Player::setPosition(float newX, float newY, int newSize) {
    x = newX;
    y = newY;
    size = newSize;
    
    rect.w = rect.h = size;
    rect.x = (int)x;
    rect.y = (int)y;
}

void Player::update(float deltaTime) {
    x += velocityX * speed * deltaTime;
    y += velocityY * speed * deltaTime;
    
    rect.x = (int)x;
    rect.y = (int)y;
}

void Player::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
