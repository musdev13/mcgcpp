#ifndef Player_hpp
#define Player_hpp

#include "SDL2/SDL.h"
#include <string>

class Player {
public:
    Player();
    ~Player();
    
    void setPosition(float newX, float newY, int size);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer) const;
    
    void setVelocity(float vx, float vy) { velocityX = vx; velocityY = vy; }
    float getX() const { return x; }
    float getY() const { return y; }

    bool loadSprite(SDL_Renderer* renderer, const std::string& path);
    void setAnimation(const std::string& state);
    void setSpeed(float newSpeed) { speed = newSpeed; }

    enum class Direction {
        DOWN,
        LEFT,
        UP,
        RIGHT
    };

private:
    SDL_Rect rect;
    float x, y;
    float velocityX, velocityY;
    float speed;
    SDL_Color color;
    int size;

    SDL_Texture* spriteSheet;
    SDL_Rect srcRect;  // Source rectangle for sprite animation
    int frameWidth;
    int frameHeight;
    int currentFrame;
    float frameTimer;
    float frameDuration;
    std::string currentState;
    
    Direction currentDirection;
    Direction lastDirection;
    
    float scale;  // Add this new property

    void updateAnimation(float deltaTime);
    void updateAnimationState();
    void setDirection(float dx, float dy);
};

#endif
