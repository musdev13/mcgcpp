#ifndef Player_hpp
#define Player_hpp

#include "SDL2/SDL.h"
#include <string>

class Player {
public:
    Player();
    
    void setPosition(float newX, float newY, int size);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer) const;
    
    void setVelocity(float vx, float vy) { velocityX = vx; velocityY = vy; }
    float getX() const { return x; }
    float getY() const { return y; }

private:
    SDL_Rect rect;
    float x, y;
    float velocityX, velocityY;
    float speed;
    SDL_Color color;
    int size;
};

#endif
