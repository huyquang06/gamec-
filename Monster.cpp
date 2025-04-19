#include "Monster.h"

Monster::Monster(SDL_Texture* texture, int frameCount, SDL_FPoint pos)
    : mTexture(texture), mFrameCount(frameCount), position(pos), mFrame(0), mLastFrameTime(SDL_GetTicks())
{
    int texW, texH;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    mFrameWidth = texW / frameCount;
    mFrameHeight = texH;
}

void Monster::Render(SDL_Renderer* renderer, SDL_Rect& camera) {
    Uint32 now = SDL_GetTicks();
    if (now > mLastFrameTime + 200) {
        mFrame = (mFrame + 1) % mFrameCount;
        mLastFrameTime = now;
    }

    SDL_Rect srcRect = { mFrame * mFrameWidth, 0, mFrameWidth, mFrameHeight };
    SDL_Rect dstRect = { (int)(position.x - camera.x), (int)(position.y - camera.y), mFrameWidth, mFrameHeight };
    SDL_RenderCopy(renderer, mTexture, &srcRect, &dstRect);
}

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height) {
    std::vector<SDL_FPoint> positions;

    int numZones = 5;  // bạn có thể chỉnh số vùng tuỳ ý
    int monstersPerZone = count / numZones;
    const int minDistance = 200;

    int zoneWidth = max_map_x / numZones;

    for (int zone = 0; zone < numZones; ++zone) {
        int zoneStart = zone * zoneWidth;
        int zoneEnd = zoneStart + zoneWidth;

        int attempts = 0;
        int created = 0;
        while (created < monstersPerZone && attempts < 20 * monstersPerZone) {
            attempts++;

            float x = zoneStart + rand() % (zoneEnd - zoneStart);
            float y = getGroundLevel(x, width, height);

            bool tooClose = false;
            for (const auto& pos : positions) {
                if (fabs(pos.x - x) < minDistance) {
                    tooClose = true;
                    break;
                }
            }

            if (!tooClose) {
                positions.push_back({ x, y });
                created++;
            }
        }
    }

    return positions;
}


std::vector<Monster> InitMonsters(SDL_Renderer* renderer, const char* texturePath, int frameCount, const std::vector<SDL_FPoint>& positions) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, texturePath);
    std::vector<Monster> monsters;
    if (!texture) {
        SDL_Log("Failed to load monster texture: %s", IMG_GetError());
        return monsters;
    }

    for (auto& pos : positions) {
        monsters.emplace_back(texture, frameCount, pos);
    }
    return monsters;
}

void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera) {
    for (auto& monster : monsters) {
        monster.Render(renderer, camera);
    }
}
