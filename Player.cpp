#include "Player.hpp"

Player::Player() : 
    targetRow(0),
    targetCol(0),
    currentRow(0), 
    currentCol(0),
    x(0),
    y(0),
    color{0, 0, 255, 255},
    moveSpeed(1.0f / 30.0f), // Для движения за 1 секунду (30 FPS)
    gridSize(0)
{
    rect = {0, 0, 0, 0};
}

void Player::setPosition(int row, int col, int gSize) {
    currentRow = targetRow = row;
    currentCol = targetCol = col;
    gridSize = gSize;
    
    x = col * gridSize;
    y = row * gridSize;
    
    rect.w = rect.h = gridSize;
    rect.x = (int)x;
    rect.y = (int)y;
}

bool Player::isMoving() const {
    return (int)x != targetCol * gridSize || (int)y != targetRow * gridSize;
}

void Player::move(int newRow, int newCol) {
    if (isMoving()) return;
    targetRow = newRow;
    targetCol = newCol;
}

void Player::update() {
    float targetX = targetCol * gridSize;
    float targetY = targetRow * gridSize;
    
    float pixelsPerFrame = (gridSize / 10.0f); // Замедляем: 8 кадров на клетку вместо 6
    
    bool reachedX = std::abs(x - targetX) < 1.0f;
    bool reachedY = std::abs(y - targetY) < 1.0f;
    
    if (reachedX) {
        x = targetX;
    } else if (x < targetX) {
        x = std::min(x + pixelsPerFrame, targetX);
    } else {
        x = std::max(x - pixelsPerFrame, targetX); // Исправлено: убран лишний targetX
    }
    
    if (reachedY) {
        y = targetY;
    } else if (y < targetY) {
        y = std::min(y + pixelsPerFrame, targetY);
    } else {
        y = std::max(y - pixelsPerFrame, targetY);
    }
    
    rect.x = (int)x;
    rect.y = (int)y;
}

void Player::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
