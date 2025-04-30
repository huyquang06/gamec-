#include "Monster.h"
#include "CommonFunc.h"
#include "Player.h"

bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

Fireball::Fireball(SDL_Renderer* renderer, float x, float y, float velocityX) : x(x), y(y), velocityX(velocityX)
{
    texture = IMG_LoadTexture(renderer, "image/FIREBALL.png");
    if(!texture)
    {
        SDL_Log("Failed to load fireball texture: %s", IMG_GetError());
        width = 0;
        height = 0;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }
}

void Fireball::Update(const int map[MAP_HEIGHT][MAP_WIDTH])
{
    x+= velocityX;  // di chuyen fireball theo huong

    int row = static_cast<int>(y / TILE_SIZE);
    int col = static_cast<int>(x / TILE_SIZE);

    if(row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH)
    {
        if(map[row][col] == 1 || map[row][col] == 2)
        {
            x = -1000;
        }
    }
}

void Fireball::Render(SDL_Renderer* renderder, SDL_Rect& camera)
{
    SDL_Rect dstRect = {(int)(x - camera.x), (int)(y - camera.y), width, height};
    SDL_RenderCopy(renderder, texture, NULL, &dstRect);
}

SDL_Rect Fireball::GetBoundingBox() const
{
    return {(int)x, (int)y, width, height};
}

bool Fireball::isOutOfBounds() const
{
    return x < 0 || x > MAP_WIDTH * TILE_SIZE;
}

Monster::Monster(SDL_Texture* idleTexture, SDL_Texture* idleLeftTexture, SDL_Texture* attackTexture, SDL_Texture* attackLeftTexture, int idleCount, int attackCount, SDL_Renderer* renderer, const int map[MAP_HEIGHT][MAP_WIDTH])
    : m_idleTexture(idleTexture),
      m_idleLeftTexture(idleLeftTexture),
      m_attackTexture(attackTexture),
      m_attackLeftTexture(attackLeftTexture),
      m_idleCount(idleCount),
      m_attackCount(attackCount),
      position({0,0}),
      mFrame(0),
      mLastFrameTime(SDL_GetTicks()),
      mState(State::IDLE),
      fRenderer(renderer),
      mMap(map),
      justAttack(false),
      hitCount(0),
      markedForDeletion(false),
      facingRight(true)
{
    int idleTexW, texH;
    SDL_QueryTexture(m_idleTexture, NULL, NULL, &idleTexW, &texH);
    mFrameWidth = idleTexW / idleCount;
    mFrameHeight = texH;

    int attackTexW;
    SDL_QueryTexture(m_attackTexture, NULL, NULL, &attackTexW, &texH);
    mA_FrameWidth = attackTexW / attackCount;

    m_hurtTexture = IMG_LoadTexture(renderer, "image/HURT_RIGHT.png");
    m_hurtLeftTexture = IMG_LoadTexture(renderer, "image/HURT.png");
    m_hurtCount = 4;
    int hurtTexW;
    SDL_QueryTexture(m_hurtTexture, NULL, NULL, &hurtTexW, &texH);
    mH_FrameWidth = hurtTexW / m_hurtCount;

    m_deathTexture = IMG_LoadTexture(renderer, "image/DEATH_RIGHT.png");
    m_deathLeftTexture = IMG_LoadTexture(renderer, "image/DEATH.png");
    m_deathCount = 6;
    int deathTexW;
    SDL_QueryTexture(m_deathTexture,  NULL, NULL, &deathTexW, &texH);
    mD_FrameWidth = deathTexW / m_deathCount;
}

void Monster::takeHit()
{
    hitCount++;
    mFrame = 0; // reset frame de chay lai
    mLastFrameTime = SDL_GetTicks();

    if(hitCount == 1)
    {
        mState = State::HURT;
    }
    else if(hitCount >= 2)
    {
        mState = State::DEATH;
    }
}

void Monster::Update(const Player& player)
{
    if(mState == State::HURT || mState == State::DEATH)
    {
        return;
    }

    SDL_Rect monsterBox = GetBoundingBox();
    SDL_Rect playerBox = player.GetBoundingBox();

    State previousState  = mState;

    if(monsterCollision(monsterBox, playerBox))
    {
        mState = State::ATTACK;
        std::cout << "Collision detected, switching to ATTACK state\n";
        const_cast<Player&>(player).IncreaseRespawnCount();
        const_cast<Player&>(player).Respawn();
    }
    else if(mState == State::HURT || mState == State::DEATH)
    {

    }
    else
    {
        mState = State::IDLE;
        justAttack = false;
    }

    float playerX = player.getX();
    facingRight = (playerX >= position.x);

    float distance = std::abs(playerX - position.x);
    const float attackDistance = 4 * TILE_SIZE;

    static Uint32 lastFireballTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    const Uint32 fireballCooldown = 1000;

    if(distance <= attackDistance && (currentTime - lastFireballTime) >= fireballCooldown && mState != State::HURT && mState != State::DEATH)
    {
        float velocityX = (playerX < position.x) ? -2.0f : 2.0f; // fireball di chuyen ve phia nhan vat
        fireballs.emplace_back(fRenderer, position.x , position.y - mFrameHeight , velocityX);
        lastFireballTime = currentTime;
    }

    for(auto it = fireballs.begin(); it != fireballs.end();)
    {
        it->Update(mMap);    // truyen vao map de check va cham
        SDL_Rect fireballBox = it->GetBoundingBox();
        if(monsterCollision(fireballBox, playerBox))
        {
            Player::CharacterState playerState = player.getState();
            if(playerState == Player::DEFEND_RIGHT || playerState == Player::DEFEND_LEFT)
            {
                it = fireballs.erase(it); // xoa fireball neu bi chan
            }
            else
            {
                const_cast<Player&>(player).IncreaseRespawnCount();
                const_cast<Player&>(player).Respawn();
                it = fireballs.erase(it); // xoa fireball khi va cham voi nhan vat
            }
        }
        else if(it->isOutOfBounds())
        {
            it = fireballs.erase(it); // xoa fireball neu ra khoi man hinh
        }
        else
        {
            ++it;
        }
    }
}

void Monster::Render(SDL_Renderer* renderer, SDL_Rect& camera)
{
    Uint32 now = SDL_GetTicks();
    Uint32 frameDelay = (mState == State::HURT || mState == State::DEATH) ? 500 : 200; // 500ms cho hurt va death, 200ms cho idle va attack

    if (now > mLastFrameTime + 200)
    {
        mFrame = (mFrame + 1);
        mLastFrameTime = now;

        if(mState == State::IDLE)
        {
            mFrame %= m_idleCount;
        }
        else if(mState == State::ATTACK)
        {
            mFrame %= m_attackCount;
        }
        else  if(mState == State::HURT)
        {
            if(mFrame >= m_hurtCount)
            {
                mState = State::IDLE;
                mFrame = 0;
            }
        }
            else if(mState == State::DEATH)
            {
                if(mFrame >= m_deathCount)
                {
                    markedForDeletion = true;
                    return;
                }
            }
        }


    SDL_Texture* currentTexture = (mState == State::IDLE) ? (facingRight ? m_idleTexture : m_idleLeftTexture) :
                                  (mState == State::ATTACK) ? (facingRight ? m_attackTexture : m_attackLeftTexture) :
                                  (mState == State::HURT) ? (facingRight ? m_hurtTexture : m_hurtLeftTexture) : (facingRight ? m_deathTexture : m_deathLeftTexture);

    int frameWidth = (mState == State::IDLE) ? mFrameWidth :
                     (mState == State::ATTACK) ? mA_FrameWidth :
                     (mState == State::HURT) ? mH_FrameWidth : mD_FrameWidth;

    SDL_Rect srcRect = { mFrame * frameWidth, 0, frameWidth, mFrameHeight };
    SDL_Rect dstRect = { (int)(position.x - camera.x), (int)(position.y - camera.y - mFrameHeight), frameWidth, mFrameHeight };
    SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);

    for(auto& Fireball : fireballs)
    {
        Fireball.Render(renderer, camera);
    }
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


    std::vector<Monster> InitMonsters(SDL_Renderer* renderer, const std::string& idleTexturePath, const std::string& idleLeftTexturePath,
                                      const std::string& attackTexturePath, const std::string& attackLeftTexturePath, int idleCount, int attackCount,
                                      const std::vector<SDL_FPoint>& positions, const int map[MAP_HEIGHT][MAP_WIDTH])
    {
     SDL_Texture* idleTexture = IMG_LoadTexture(renderer, idleTexturePath.c_str());
     SDL_Texture* idleLeftTexture = IMG_LoadTexture(renderer, idleLeftTexturePath.c_str());
     SDL_Texture* attackTexture = IMG_LoadTexture(renderer, attackTexturePath.c_str());
     SDL_Texture* attackLeftTexture = IMG_LoadTexture(renderer, attackLeftTexturePath.c_str());
     std::vector<Monster> monsters;
    if (!idleTexture)
    {
        SDL_Log("Failed to load idle texture: %s", IMG_GetError());
        return monsters;
    }
    if (!idleLeftTexture)
    {
        SDL_Log("Failed to load idle left texture: %s", IMG_GetError());
        SDL_DestroyTexture(idleTexture);
        return monsters;
    }
    if (!attackTexture)
    {
        SDL_Log("Failed to load attack texture: %s", IMG_GetError());
        SDL_DestroyTexture(idleTexture);
        SDL_DestroyTexture(idleLeftTexture);
        return monsters;
    }
    if (!attackLeftTexture)
    {
        SDL_Log("Failed to load attack texture: %s", IMG_GetError());
        SDL_DestroyTexture(idleTexture);
        SDL_DestroyTexture(idleLeftTexture);
        SDL_DestroyTexture(attackTexture);
        return monsters;
    }

    for (auto& pos : positions)
    {
        monsters.emplace_back(idleTexture, idleLeftTexture, attackTexture, attackLeftTexture, idleCount, attackCount, renderer, map);
        monsters.back().setPosition(pos);
    }
    return monsters;
}

void RenderMonsters(SDL_Renderer* renderer, std::vector<Monster>& monsters, SDL_Rect& camera) {
    for (auto& monster : monsters) {
        monster.Render(renderer, camera);
    }
}
