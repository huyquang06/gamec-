#ifndef MONSTER_H
#define MONSTER_H

#include "CommonFunc.h"
#include <vector>
#include <string>

class Player;

bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b);

class Fireball
{
public:
    Fireball(SDL_Renderer* renderer, float x, float y, float velocityX);
    void Update();
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;
    bool isOutOfBounds() const;
private:
    SDL_Texture* texture;
    float x, y;
    float velocityX;
    int width, height;
};

class Monster {
public:
    enum class State {IDLE, ATTACK, HURT, DEATH};

    Monster(SDL_Texture* idleTexture, SDL_Texture* attackTexture,
            int idleCount, int attackCount, SDL_Renderer* renderer);

    void Update(const Player& player);  // trang thai monster
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;
    void takeHit();
    bool isDelete() const { return markedForDeletion; }
    void setPosition(const SDL_FPoint& pos) { position = pos; }

private:
    SDL_Texture* m_idleTexture;
    SDL_Texture* m_attackTexture;
    SDL_Texture* m_hurtTexture;
    SDL_Texture* m_deathTexture;

    int m_idleCount;
    int m_attackCount;
    int m_hurtCount;
    int m_deathCount;

    SDL_FPoint position;
    int mFrame;
    int mFrameWidth;
    int mFrameHeight;
    int mA_FrameWidth;
    int mH_FrameWidth;
    int mD_FrameWidth;

    Uint32 mLastFrameTime;
    State mState;

    SDL_Renderer* fRenderer; // tao fireball
    std::vector<Fireball> fireballs;
    bool justAttack;  // bien kiem soat viec ban fireball chi 1 lan moi lan attack
    int hitCount;
    bool markedForDeletion;
};

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height);

std::vector<Monster> InitMonsters(
    SDL_Renderer* renderer,
    const std::string& idleTexturePath,
    const std::string& attackTexturePath,
    int idleCount,
    int attackCount,
    const std::vector<SDL_FPoint>& positions);

void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera);

#endif
