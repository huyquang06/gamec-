#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "CommonFunc.h"
#include "Monster.h"
#include "Bullet.h"
#include <vector>

class Player
{
    public:
    enum CharacterState
{
    IDLE_RIGHT, IDLE_LEFT, WALKING_RIGHT, WALKING_LEFT, ATTACK_RIGHT, ATTACK_LEFT
};

    Player(SDL_Renderer* renderer, float start_x, float start_y);
    ~Player();

    void handleInput(SDL_Event& event);
    void Update(SDL_Rect& camera, std::vector<Monster>& monsters);
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    float getX() const { return x;}
    float getY() const { return y;}
    bool isGameOver() const;
    int getRespawnCount() const;
    SDL_Rect GetBoundingBox() const;
    void IncreaseRespawnCount();
    CharacterState getState() const {return currentState;}
    void Respawn();

    void setInvincibility(Uint32 duration);
    bool isInvincible() const;

    bool isFacingRight() const { return facingRight; }
    int getAttackFrameWidth() const { return attackFrameWidth; }
    const std::vector<Bullet>& getBullets() const { return bullets; }
    std::vector<Bullet>& getBulletsNonConst() { return bullets; }

private:
    float x,y;
    float velocity, speed;
    float jumpForce, gravity;
    bool moveLeft, moveRight, facingRight, isJumping;
    int frameWidth, frameHeight;
    int idleframeWidth, idleframeHeight;
    int attackFrameWidth, attackFrameHeight;
    int walkingFrame, idleFrame, attackFrame;
    int respawnCount;
    int defendCoolDown;
    bool gameOver;
    bool hasShot;

    Uint32 frameTimer;
    CharacterState currentState;

    SDL_Texture* walkRTexture;
    SDL_Texture* walkLTexture;
    SDL_Texture* idleRTexture;
    SDL_Texture* idleLTexture;
    SDL_Texture* attackRTexture;
    SDL_Texture* attackLTexture;
    SDL_Rect spriteClip;
    SDL_Rect charRect;

    SDL_Renderer* renderer;
    std::vector<Bullet> bullets;

    bool isInvincibleState;
    Uint32 invincibilityTimer;
    Uint32 invincibilityDuration;

    int bulletCount;
    Uint32 lastShortTime;
    bool inCooldown;

private:
    void updateAnimation();
    bool checkCollision_x(float new_x, float y);
    bool checkCollision_y(float x, float new_y);
    bool isOnGround();
    float find_RespawnPoint();
};

#endif // PLAYER_H_INCLUDED
