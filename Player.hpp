#ifndef Player_hpp
#define Player_hpp

#include "SDL2/SDL.h"
#include <string>

class SceneManager;  // Forward declaration

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
    void setMovementEnabled(bool enabled) { movementEnabled = enabled; }
    bool loadSprite(SDL_Renderer* renderer, const std::string& path);
    void setAnimation(const std::string& state);
    void setSpeed(float newSpeed) { speed = newSpeed; }
    void setCollisionChecker(const SceneManager* manager) { sceneManager = manager; }
    int getSize() const { return size; }
    float getScale() const { return scale; }

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
    float scale;
    bool movementEnabled = true;

    const SceneManager* sceneManager = nullptr;
    bool checkCollision(float newX, float newY) const;
    void updateAnimation(float deltaTime);
    void setDirection(float dx, float dy);
};

#endif
