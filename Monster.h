#ifndef MONSTER_H
#define MONSTER_H

#include "CommonFunc.h"
#include <vector>
#include <string>

class Player;
bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b);

class Monster {
public:
    enum class State {IDLE, ATTACK};

    Monster(SDL_Texture* idleTexture, SDL_Texture* attackTexture, int idleCount, int attackCount, SDL_FPoint pos);
    void Update(const Player& player);  // trang thai monster
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;

private:
    SDL_Texture* m_idleTexture;
    SDL_Texture* m_attackTexture;
    int m_idleCount;
    int m_attackCount;
    SDL_FPoint position;

    int mFrame;
    int mFrameWidth;
    int mFrameHeight;
    int mA_FrameWidth;

    Uint32 mLastFrameTime;
    State mState;

};

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height);
std::vector<Monster> InitMonsters(SDL_Renderer* renderer, const std::string& idleTexturePath, const std::string& attackTexturePath, int idleCount, int attackCount, const std::vector<SDL_FPoint>& positions);
void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera);

#endif
