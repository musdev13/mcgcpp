#ifndef Player_hpp
#define Player_hpp

#include "SDL2/SDL.h"
#include <string>

class Player {
public:
    Player();
    
    void setPosition(int row, int col, int gridSize);
    bool isMoving() const;
    void move(int newRow, int newCol);
    void update();
    void render(SDL_Renderer* renderer) const;
    
    int getCurrentRow() const { return currentRow; }
    int getCurrentCol() const { return currentCol; }
    int getTargetRow() const { return targetRow; }
    int getTargetCol() const { return targetCol; }

private:
    SDL_Rect rect;
    int targetRow;
    int targetCol;
    int currentRow;
    int currentCol;
    float x, y;
    SDL_Color color;
    float moveSpeed;
    int gridSize;
};

#endif
