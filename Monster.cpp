#include "Monster.h"
#include "CommonFunc.h"
#include "Player.h"

bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

Monster::Monster(SDL_Texture* idleTexture, SDL_Texture* attackTexture, int idleCount, int attackCount, SDL_FPoint pos)
    : m_idleTexture(idleTexture),
      m_attackTexture(attackTexture),
      m_idleCount(idleCount),
      m_attackCount(attackCount),
      position(pos),
      mFrame(0),
      mLastFrameTime(SDL_GetTicks()),
      mState(State::IDLE)
{
    int texW, texH;
    SDL_QueryTexture(m_idleTexture, NULL, NULL, &texW, &texH);
    mFrameWidth = texW / idleCount;
    mFrameHeight = texH;

    int attackTexW;
    SDL_QueryTexture(m_attackTexture, NULL, NULL, &attackTexW, &texH);
    mA_FrameWidth = attackTexW / attackCount;
}


void Monster::Update(const Player& player)
{
    SDL_Rect monsterBox = GetBoundingBox();
    SDL_Rect playerBox = player.GetBoundingBox();

    if(monsterCollision(monsterBox, playerBox))
    {
        mState = State::ATTACK;
        std::cout << "Collision detected, switching to ATTACK state\n";
    }
    else
    {
        mState = State::IDLE;
    }
}

void Monster::Render(SDL_Renderer* renderer, SDL_Rect& camera) {
    Uint32 now = SDL_GetTicks();
    if (now > mLastFrameTime + 200) {
        mFrame = (mFrame + 1) % (mState == State::IDLE ? m_idleCount : m_attackCount);
        mLastFrameTime = now;
    }

    SDL_Texture* currentTexture = (mState == State::IDLE) ? m_idleTexture : m_attackTexture;
    int frameWidth = (mState == State::IDLE) ? mFrameWidth : mA_FrameWidth;
    SDL_Rect srcRect = { mFrame * mFrameWidth, 0, mFrameWidth, mFrameHeight };
    SDL_Rect dstRect = { (int)(position.x - camera.x), (int)(position.y - camera.y - mFrameHeight), mFrameWidth, mFrameHeight };
    SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);
}

SDL_Rect Monster::GetBoundingBox() const
{
    return {(int)position.x, (int)(position.y - mFrameHeight), mFrameWidth, mFrameHeight};
}

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height) {
    std::vector<SDL_FPoint> positions;

    int numZones = 5;
    int monstersPerZone = count / numZones;
    const int minDistance = 250;

    int zoneWidth = (max_map_x - 280) / numZones;

    for (int zone = 0; zone < numZones; ++zone) {
        int zoneStart = zone * zoneWidth;
        int zoneEnd = zoneStart + zoneWidth;

        int attempts = 0;
        int created = 0;
        while (created < monstersPerZone && attempts < 20 * monstersPerZone)
        {
            attempts++;

            float x = zoneStart + rand() % (zoneEnd - zoneStart);
            float y = getGroundLevel(x, width, height, false);

            bool tooClose = false;
            for (const auto& pos : positions)
            {
                if (fabs(pos.x - x) < minDistance)
                {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose)
            {
                positions.push_back({ x, y });
                created++;
            }
        }
    }
    return positions;
}


    std::vector<Monster> InitMonsters(SDL_Renderer* renderer, const std::string& idleTexturePath, const std::string& attackTexturePath, int idleCount, int attackCount, const std::vector<SDL_FPoint>& positions)
    {
     SDL_Texture* idleTexture = IMG_LoadTexture(renderer, idleTexturePath.c_str());
     SDL_Texture* attackTexture = IMG_LoadTexture(renderer, attackTexturePath.c_str());
     std::vector<Monster> monsters;
    if (!idleTexture)
    {
        SDL_Log("Failed to load idle texture: %s", IMG_GetError());
        return monsters;
    }
    if (!attackTexture)
    {
        SDL_Log("Failed to load attack texture: %s", IMG_GetError());
        SDL_DestroyTexture(idleTexture);
        return monsters;
    }
    for (auto& pos : positions)
    {
        monsters.emplace_back(idleTexture, attackTexture, idleCount, attackCount, pos);
    }
    return monsters;
}

void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera) {
    for (auto& monster : monsters) {
        monster.Render(renderer, camera);
    }
}
