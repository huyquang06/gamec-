#ifndef MONSTER_H
#define MONSTER_H

#include "CommonFunc.h"
#include <vector>
#include <string>

class Player;
class Bullet; // Forward declaration cho Bullet

bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b);

class Fireball
{
public:
    Fireball(SDL_Renderer* renderer, float x, float y, float velocityX);
    void Update(const int map[MAP_HEIGHT][MAP_WIDTH]);
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;
    bool isOutOfBounds() const;

private:
    SDL_Texture* textureLeft;
    SDL_Texture* textureRight;
    float x, y;
    float velocityX;
    int width, height;
};

class Monster {
public:
    enum class State {IDLE, ATTACK, HURT, DEATH};

    Monster(SDL_Texture* idleTexture, SDL_Texture* idleLeftTexture, SDL_Texture* attackTexture, SDL_Texture* attackLeftTexture,
            int idleCount, int attackCount, SDL_Renderer* renderer, const int map[MAP_HEIGHT][MAP_WIDTH]);

    void Update(const Player& player);  // Trạng thái monster
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;
    void TakeDamage(int damage);        // Phương thức để nhận sát thương
    void takeHit();                     // Phương thức cũ để nhận đòn đánh
    bool isMarkedForDeletion() const { return markedForDeletion; }
    void setPosition(const SDL_FPoint& pos) { position = pos; }

private:
    SDL_Texture* m_idleTexture;
    SDL_Texture* m_idleLeftTexture;
    SDL_Texture* m_attackTexture;
    SDL_Texture* m_attackLeftTexture;
    SDL_Texture* m_hurtTexture;
    SDL_Texture* m_hurtLeftTexture;
    SDL_Texture* m_deathTexture;
    SDL_Texture* m_deathLeftTexture;

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

    SDL_Renderer* fRenderer; // Tạo fireball
    const int (*mMap)[MAP_WIDTH];
    std::vector<Fireball> fireballs;
    bool justAttack;  // Biến kiểm soát việc bắn fireball chỉ 1 lần mỗi lần attack
    int hitCount;
    bool markedForDeletion;
    bool facingRight;
};

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height);

std::vector<Monster> InitMonsters(
    SDL_Renderer* renderer,
    const std::string& idleTexturePath,
    const std::string& idleLeftTexturePath,
    const std::string& attackTexturePath,
    const std::string& attackLeftTexturePath,
    int idleCount,
    int attackCount,
    const std::vector<SDL_FPoint>& positions,
    const int map[MAP_HEIGHT][MAP_WIDTH]);

void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera);


#endif
