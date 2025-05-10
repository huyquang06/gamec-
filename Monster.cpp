#include "Monster.h"
#include "CommonFunc.h"
#include "Player.h"
#include "Bullet.h"

bool monsterCollision(const SDL_Rect& a, const SDL_Rect& b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

Fireball::Fireball(SDL_Renderer* renderer, float x, float y, float velocityX) : x(x), y(y), velocityX(velocityX)
{
    textureLeft = IMG_LoadTexture(renderer, "image/FIREBALL.png");
    textureRight = IMG_LoadTexture(renderer, "image/FIREBALL_REVERSE.png");
    if(!textureLeft || !textureRight)
    {
        SDL_Log("Failed to load fireball texture: %s", IMG_GetError());
        width = 0;
        height = 0;
    }
    else
    {
        SDL_QueryTexture(velocityX >= 0 ? textureRight : textureLeft, NULL, NULL, &width, &height);
    }
}

void Fireball::Update(const int map[MAP_HEIGHT][MAP_WIDTH])
{
    x += velocityX;

    int row = static_cast<int>(y / TILE_SIZE);
    int col = static_cast<int>(x / TILE_SIZE);

    if(row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH)
    {
        if(map[row][col] == 1 || map[row][col] == 3)
        {
            x = -1000;
        }
    }
}

void Fireball::Render(SDL_Renderer* renderer, SDL_Rect& camera)
{
    SDL_Texture* currentTexture = (velocityX >= 0) ? textureRight : textureLeft;
    SDL_Rect dstRect = {(int)(x - camera.x), (int)(y - camera.y), width, height};
    SDL_RenderCopy(renderer, currentTexture, NULL, &dstRect);
}

SDL_Rect Fireball::GetBoundingBox() const
{
    const int offsetX = 5;
    const int offsetY = 5;
    const int shrinkW = 5; // Giảm shrink để tăng vùng va chạm
    const int shrinkH = 5;
    return {(int)x + offsetX, (int)y + offsetY, width - shrinkW, height - shrinkH};
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

    m_hurtTexture = IMG_LoadTexture(renderer, "image/MONSTER_HURT_RIGHT.png");
    m_hurtLeftTexture = IMG_LoadTexture(renderer, "image/MONSTER_HURT_LEFT.png");
    m_hurtCount = 4;
    int hurtTexW;
    SDL_QueryTexture(m_hurtTexture, NULL, NULL, &hurtTexW, &texH);
    mH_FrameWidth = hurtTexW / m_hurtCount;

    m_deathTexture = IMG_LoadTexture(renderer, "image/DEATH_RIGHT.png");
    m_deathLeftTexture = IMG_LoadTexture(renderer, "image/DEATH.png");
    m_deathCount = 6;
    int deathTexW;
    SDL_QueryTexture(m_deathTexture, NULL, NULL, &deathTexW, &texH);
    mD_FrameWidth = deathTexW / m_deathCount;
}

void Monster::TakeDamage(int damage)
{
    hitCount += damage;
    mFrame = 0;
    mLastFrameTime = SDL_GetTicks();
    std::cout << "Monster hit, hitCount=" << hitCount << "\n";

    if(hitCount == 1)
    {
        mState = State::HURT;
    }
    else if(hitCount >= 2)
    {
        mState = State::DEATH;
        std::cout << "Monster marked for death\n";
    }
}

void Monster::Update( Player& player)
{
    for(auto it = fireballs.begin(); it != fireballs.end();)
    {
    it->Update(mMap);
    SDL_Rect fireballBox = it->GetBoundingBox();
    SDL_Rect playerBox = player.GetBoundingBox();

    if (monsterCollision(fireballBox, playerBox))
    {
        if (!player.isInvincible())
        {
            const_cast<Player&>(player).IncreaseRespawnCount();
            const_cast<Player&>(player).Respawn();
        }
        it = fireballs.erase(it);
    }
    else if(it->isOutOfBounds())
    {
        it = fireballs.erase(it);
    }
    else
    {
        ++it;
    }
    }


    if(mState == State::HURT || mState == State::DEATH)  {return;}

    SDL_Rect monsterBox = GetBoundingBox();
    float groundY = getGroundLevel(position.x, mFrameWidth, mFrameHeight, false, mMap);
    if (position.y < groundY) {
        position.y = groundY;
    }

    if (player.getState() == Player::ATTACK_RIGHT || player.getState() == Player::ATTACK_LEFT)
    {
        SDL_Rect attackBox = player.GetBoundingBox();
        if (player.isFacingRight()) {
            attackBox.x += player.getAttackFrameWidth() / 2;
        } else {
            attackBox.x -= player.getAttackFrameWidth() / 2;
        }
        if (monsterCollision(attackBox, monsterBox))
        {
            TakeDamage(1);
        }
    }


    auto& bullets = const_cast<Player&>(player).getBulletsNonConst(); // Lấy tham chiếu không const
    auto bulletIt = bullets.begin();
    while (bulletIt != bullets.end())
    {
        SDL_Rect bulletBox = bulletIt->GetBoundingBox();
        bool bulletConsumed = false;

        if (monsterCollision(bulletBox, monsterBox))
        {
            TakeDamage(2);
            bulletIt = bullets.erase(bulletIt); // Xóa Bullet sau khi va chạm với Monster
            bulletConsumed = true;
        }

        if(!bulletConsumed)
        {
            auto fireballIt = fireballs.begin();
            while (fireballIt != fireballs.end())
            {
                SDL_Rect fireballBox = fireballIt->GetBoundingBox();
                if (monsterCollision(bulletBox, fireballBox))
                {
                    fireballIt = fireballs.erase(fireballIt);
                    bulletIt = bullets.erase(bulletIt); // Xóa Bullet sau khi va chạm với Fireball
                    bulletConsumed = true;
                    break;
                }
                else
                {
                    ++fireballIt;
                }
            }
        }

        if (!bulletConsumed)
        {
            ++bulletIt;
        }
    }

    State previousState = mState;

    if(!player.isInvincible() && monsterCollision(monsterBox, player.GetBoundingBox()))
    {
        mState = State::ATTACK;
        const_cast<Player&>(player).IncreaseRespawnCount();
        const_cast<Player&>(player).Respawn();
    }
    else
    {
        mState = State::IDLE;
        justAttack = false;
    }

    if(mState != previousState)
    {
        mFrame = 0;
        mLastFrameTime = SDL_GetTicks();
    }
    float playerX = player.getX();
    facingRight = (playerX >= position.x);

    float distance = std::abs(playerX - position.x);
    const float attackDistance = 4 * TILE_SIZE;

    static Uint32 lastFireballTime = 0;

    Uint32 currentTime = SDL_GetTicks();
    const Uint32 fireballCooldown = 1000;

    if(distance <= attackDistance && (currentTime - lastFireballTime) >= fireballCooldown)
    {
        float fireballY = position.y - mFrameHeight;
        float velocityX = (playerX < position.x) ? -2.0f : 2.0f;
        fireballs.emplace_back(fRenderer, position.x, fireballY, velocityX);
        lastFireballTime = currentTime;

        mState = State::ATTACK;
        mFrame = 0;
        mLastFrameTime = SDL_GetTicks();
    }
}

void Monster::Render(SDL_Renderer* renderer, SDL_Rect& camera)
{
    Uint32 now = SDL_GetTicks();
    Uint32 frameDelay = (mState == State::HURT || mState == State::DEATH) ? 200 : 100;

    if (now > mLastFrameTime + frameDelay)
    {
        mFrame++;
        mLastFrameTime = now;

        if(mState == State::IDLE)
        {
            mFrame %= m_idleCount;
        }
        else if(mState == State::ATTACK)
        {
            mFrame %= m_attackCount;
        }
        else if(mState == State::HURT)
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
    SDL_Rect dstRect = { (int)(position.x - camera.x), (int)(position.y - mFrameHeight - camera.y), frameWidth, mFrameHeight };
    SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);

    for(auto& fireball : fireballs)
    {
        fireball.Update(mMap);
        fireball.Render(renderer, camera);
    }
}

SDL_Rect Monster::GetBoundingBox() const
{
    const int offsetX = 10;
    const int offsetY = 10;
    const int shrinkW = 10; // Giảm shrink để tăng vùng va chạm
    const int shrinkH = 10;
    return {(int)position.x + offsetX, (int)(position.y - mFrameHeight) + offsetY, mFrameWidth - shrinkW, mFrameHeight - shrinkH};
}

std::vector<SDL_FPoint> Generate_Monsters(int count, int min_x, int max_x, int max_map_x, int width, int height, const int tileMap[MAP_HEIGHT][MAP_WIDTH]) {
    std::vector<SDL_FPoint> positions;

    int numZones = 5;
    int monstersPerZone = count / numZones;
    const int minDistance = 300;

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
            float y = getGroundLevel(x, width, height, true, tileMap);

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
