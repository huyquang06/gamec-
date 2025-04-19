#ifndef MONSTER_H
#define MONSTER_H

#include "CommonFunc.h"
#include <vector>

class Monster {
public:
    Monster(SDL_Texture* texture, int frameCount, SDL_FPoint pos);
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_FPoint position;

private:
    SDL_Texture* mTexture;
    int mFrameCount;
    int mFrame;
    int mFrameWidth;
    int mFrameHeight;
    Uint32 mLastFrameTime;
};

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height);
std::vector<Monster> InitMonsters(SDL_Renderer* renderer, const char* texturePath, int frameCount, const std::vector<SDL_FPoint>& positions);
void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera);

#endif
