#include "Player.h"

Player::Player(SDL_Renderer* renderer, float start_x, float start_y)
{
    x = start_x; y = start_y;
    velocity = 0.0f; speed = 1.5f;
    gravity = 0.05f; jumpForce = 3.0f;

    moveLeft = false; moveRight = false;
    facingRight = true;
    isJumping = false;
    walkingFrame = 0;
    idleFrame = 0;
    attackFrame = 0;
    frameTimer = SDL_GetTicks();
    currentState = IDLE_RIGHT;

    respawnCount = 0;
    gameOver = false;

    lastShortTime = 0;
    inCooldown  = 0;
    bulletCount = 0;

    walkRTexture = IMG_LoadTexture(renderer, "image/RUN.png");
    walkLTexture = IMG_LoadTexture(renderer, "image/RUN_NGUOC.png");
    idleLTexture = IMG_LoadTexture(renderer, "image/IDLE_LEFT.png");
    idleRTexture = IMG_LoadTexture(renderer, "image/IDLE_RIGHT.png");
    attackRTexture = IMG_LoadTexture(renderer, "image/MAIN_ATTACK.png");
    attackLTexture = IMG_LoadTexture(renderer, "image/MAIN_ATTACK_LEFT.png");

    // Lấy kích thước nhân vật khi di chuyển
    int textureWidth, textureHeight;
    if (walkRTexture) {
        SDL_QueryTexture(walkRTexture, NULL, NULL, &textureWidth, &textureHeight);
        frameWidth = textureWidth / WALKING_FRAMES;
        frameHeight = textureHeight;
    } else {
        frameWidth = 0;
        frameHeight = 0;
    }

    int idleTextureWidth, idleTextureHeight;
    if (idleRTexture) {
        SDL_QueryTexture(idleRTexture, NULL, NULL, &idleTextureWidth, &idleTextureHeight);
        idleframeWidth = idleTextureWidth / IDLE_FRAME;
        idleframeHeight = idleTextureHeight;
    } else {
        idleframeWidth = 0;
        idleframeHeight = 0;
    }

    int attackTextureWidth, attackTextureHeight;
    if(attackRTexture)
    {
        SDL_QueryTexture(attackRTexture, NULL, NULL, &attackTextureWidth, &attackTextureHeight);
        attackFrameWidth = attackTextureWidth / ATTACK_FRAMES;
        attackFrameHeight = attackTextureHeight;
    }
    else
    {
        attackFrameWidth = 0;
        attackFrameHeight = 0;
    }

    spriteClip = {0, 0, idleframeWidth, idleframeHeight};
    charRect = {(int)x, (int)y, idleframeWidth, idleframeHeight};

    this->renderer = renderer;
}

Player::~Player()
{
    SDL_DestroyTexture(walkRTexture);
    SDL_DestroyTexture(walkLTexture);
    SDL_DestroyTexture(idleRTexture);
    SDL_DestroyTexture(idleLTexture);
    SDL_DestroyTexture(attackRTexture);
    SDL_DestroyTexture(attackLTexture);
}

void Player::handleInput(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_a) { moveLeft = true; facingRight = false; }
        if (event.key.keysym.sym == SDLK_d) { moveRight = true; facingRight = true; }
        if (event.key.keysym.sym == SDLK_SPACE && !isJumping) {
            isJumping = true;
            velocity = -jumpForce;
        }
    }
    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_a) moveLeft = false;
        if (event.key.keysym.sym == SDLK_d) moveRight = false;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
    {
        currentState = facingRight ? ATTACK_RIGHT : ATTACK_LEFT;

        if(bulletCount < 3 && !inCooldown)
        {
            float bulletY = y + frameHeight / 5;
            float velocityX = facingRight ? 2.0f : -2.0f;
            bullets.emplace_back(renderer, x + (facingRight ? frameWidth / 2 : -frameWidth /2 ), bulletY, velocityX);
            bulletCount++;
            lastShortTime = SDL_GetTicks();
        }

    }
}

bool Player::checkCollision_x(float new_x, float y)
{
    float left_mar = new_x + COLLISION_MARGIN;
    float right_mar = new_x + frameWidth - COLLISION_MARGIN - 1;

    int left = static_cast<int>(left_mar) / TILE_SIZE;
    int right = static_cast<int>(right_mar) / TILE_SIZE;
    int top = static_cast<int>(y) / TILE_SIZE;
    int bottom = static_cast<int>(y + frameHeight - 1) / TILE_SIZE;

    for (int row = top; row <= bottom; row++) {
        if ((left >= 0 && left < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT && is_solid(tileMap[row][left])) ||
            (right >= 0 && right < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT && is_solid(tileMap[row][right]))){
            return true;
        }
    }
    return false;
}

bool Player::checkCollision_y(float x, float new_y) {
    int left = static_cast<int>(x) / TILE_SIZE;
    int right = static_cast<int>(x + frameWidth - 1) / TILE_SIZE;
    int top = static_cast<int>(new_y) / TILE_SIZE;
    int bottom = static_cast<int>(new_y + frameHeight - 1) / TILE_SIZE;

    for (int col = left; col <= right; col++) {
        if ((top >= 0 && top < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH && is_solid(tileMap[top][col])) ||
            (bottom >= 0 && bottom < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH && is_solid(tileMap[bottom][col]))) {
            return true;
        }
    }
    return false;
}

bool Player::isOnGround() {
    float feet_y = y + frameHeight + 1;
    return checkCollision_y(x + 1, feet_y) ||
           checkCollision_y(x + frameWidth / 2, feet_y) ||
           checkCollision_y(x + frameWidth - 2, feet_y);
}

float Player::find_RespawnPoint() {
    int col = static_cast<int>(x / TILE_SIZE);
    while (col >= 0) {
        for (int row = 0; row < MAP_HEIGHT - 1; row++) {
            if (!is_solid(tileMap[row][col]) && is_solid(tileMap[row + 1][col])) {
                return col * TILE_SIZE;
            }
        }
        col--;
    }
    return 0.0f;
}

void Player::updateAnimation()
{
    if(SDL_GetTicks() > frameTimer)
    {
        if(currentState == WALKING_LEFT || currentState == WALKING_RIGHT)
        {
            walkingFrame = (walkingFrame + 1) % WALKING_FRAMES;
            frameTimer = SDL_GetTicks() + IDLE_FRAME_DELAY;
        }
        else if(currentState == ATTACK_LEFT || currentState == ATTACK_RIGHT)
        {
            attackFrame++;
            if(attackFrame >= ATTACK_FRAMES)
            {
                currentState = facingRight ? IDLE_RIGHT : IDLE_LEFT;
                attackFrame = 0;
            }
            frameTimer = SDL_GetTicks() + IDLE_FRAME_DELAY / 2;
        }
        else
        {
            idleFrame = (idleFrame + 1) % IDLE_FRAME;
            frameTimer = SDL_GetTicks() + IDLE_FRAME_DELAY;
        }
    }

    if(currentState == WALKING_LEFT || currentState == WALKING_RIGHT)
    {
        spriteClip.x = walkingFrame * frameWidth;
        spriteClip.y = 0;
        spriteClip.w = attackFrameWidth;
        spriteClip.h = attackFrameHeight;
    }
    else if(currentState == ATTACK_LEFT || currentState == ATTACK_RIGHT)
    {
        spriteClip.x = attackFrame * attackFrameWidth;
        spriteClip.y = 0;
        spriteClip.w = attackFrameWidth;
        spriteClip.h = attackFrameHeight;
    }
    else
    {
        spriteClip.x = idleFrame * idleframeWidth;
        spriteClip.y = 0;
        spriteClip.w = idleframeWidth;
        spriteClip.h = idleframeHeight;
    }
}

void Player::Update(SDL_Rect& camera, std::vector<Monster>& monsters)
{
    if(bulletCount >= 3 && !inCooldown)
    {
        inCooldown = true;
    }
    if(inCooldown )
    {
        Uint32 currentTime = SDL_GetTicks();
        if(currentTime - lastShortTime >= 3000)
        {
            bulletCount = 0;
            inCooldown = false;
        }
    }
    if(currentState != ATTACK_LEFT && currentState != ATTACK_RIGHT )
    {
        if (isJumping)
        {
            currentState = facingRight ? IDLE_RIGHT : IDLE_LEFT;
            if (moveLeft) currentState = WALKING_LEFT;
            if (moveRight) currentState = WALKING_RIGHT;
        }
        else
        {
            currentState = moveLeft ? WALKING_LEFT :
                           moveRight ? WALKING_RIGHT :
                           facingRight ? IDLE_RIGHT : IDLE_LEFT;
        }
    }

    if(moveLeft && !checkCollision_x(x-speed,y)) x -= speed;

    if(moveRight)
    {
        float new_x = x + speed;
        if(!checkCollision_x(new_x, y))
        {
            x = new_x;
        }
        else
        {
            int right_tile = static_cast<int>((new_x + frameWidth - COLLISION_MARGIN - 1) / TILE_SIZE);
            x = right_tile * TILE_SIZE - frameWidth + COLLISION_MARGIN;
        }
    }
    // Kiểm tra biên trái, phải
    if(x < 0) x = 0;
    if(x > MAP_WIDTH * TILE_SIZE - frameWidth) x = MAP_WIDTH * TILE_SIZE - frameWidth;

    velocity += gravity;
    float new_y = y + velocity;

    bool fall_down = new_y > WINDOW_HEIGHT;
    if(fall_down)
    {
        IncreaseRespawnCount();
        if(respawnCount >= 3)
        {
            gameOver = true;
            std::cout << "TRÒ CHƠI KẾT THÚC!" << std::endl;
            return;
        }
        x = 0.0f;
        y = getGroundLevel(x, frameWidth, frameHeight, true);
        velocity = 0;
        isJumping = false;
    }
    else if(!checkCollision_y(x, new_y))
    {
        y = new_y;
    }
    else
    {
        if(velocity > 0)
        {
            y = ((y + frameHeight) / TILE_SIZE) * TILE_SIZE - frameHeight;
            isJumping = false;
        }
        else
        {
            y = (y / TILE_SIZE) * TILE_SIZE;
        }
        velocity = 0;
    }

    if(!isJumping && !isOnGround())
    {
        isJumping = true;
    }

    if(currentState == ATTACK_LEFT || currentState == ATTACK_RIGHT)
    {
        SDL_Rect attackBox = GetBoundingBox();
        if (facingRight) {
            attackBox.x += attackFrameWidth / 2;
        } else {
            attackBox.x -= attackFrameWidth / 2;
        }
        for (auto& monster : monsters) {
            SDL_Rect monsterBox = monster.GetBoundingBox();
            if (SDL_HasIntersection(&attackBox, &monsterBox)) {
                monster.TakeDamage(1);
            }
        }
    }

    // Cập nhật và xóa Bullet
    for (auto it = bullets.begin(); it != bullets.end();)
    {
        it->Update(tileMap);
        std::cout << "Bullet x: " << it->getX() << ", y: " << it->getY() << "\n";
        if (it->isOutOfBounds())
        {
            std::cout << "Bullet out of bounds, erasing\n";
            it = bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }

    camera.x = static_cast<int>(x + frameWidth / 2 - WINDOW_WIDTH / 2);
    camera.y = 0;
    if(camera.x < 0) camera.x = 0;
    if(camera.x > MAP_WIDTH * TILE_SIZE - WINDOW_WIDTH) camera.x = MAP_WIDTH * TILE_SIZE - WINDOW_WIDTH;

    updateAnimation();

    charRect.x = (int)x - camera.x;
    charRect.y = (int)y - camera.y;
    charRect.w = spriteClip.w;
    charRect.h = spriteClip.h;
}

void Player::Render(SDL_Renderer* renderer, SDL_Rect& camera)
{
    if (gameOver) return;

    SDL_Texture* currentTexture = (currentState == WALKING_LEFT) ? walkLTexture :
                                  (currentState == WALKING_RIGHT) ? walkRTexture :
                                  (currentState == IDLE_LEFT) ? idleLTexture :
                                  (currentState == ATTACK_LEFT) ? attackLTexture :
                                  (currentState == ATTACK_RIGHT) ? attackRTexture : idleRTexture;
    SDL_RenderCopy(renderer, currentTexture, &spriteClip, &charRect);

    std::cout << "Rendering " << bullets.size() << " bullets\n";
    for (auto& bullet : bullets)
    {
        bullet.Render(renderer, camera);
    }
}

bool Player::isGameOver() const
{
    return gameOver;
}

int Player::getRespawnCount() const
{
    return respawnCount;
}

SDL_Rect Player::GetBoundingBox() const
{
    const int offsetX = 5;
    const int offsetY = 5;
    const int shrinkW = 10;
    const int shrinkH = 10;
    return {(int)x + offsetX, (int)y + offsetY, spriteClip.w - shrinkW, spriteClip.h - shrinkH};
}

void Player::IncreaseRespawnCount()
{
    respawnCount++;
    if(respawnCount >= 3)
    {
        gameOver = true;
    }
    else
    {
        x = 0.0f;
        y = getGroundLevel(x, frameWidth, frameHeight, true);
        velocity = 0;
        isJumping = false;
    }
}

void Player::Respawn()
{
    x = find_RespawnPoint();
    y = getGroundLevel(x, frameWidth, frameHeight, true);
    velocity = 0;
    isJumping = false;
    currentState = facingRight ? IDLE_RIGHT : IDLE_LEFT;
}

void Player::setInvincibility(Uint32 duration)
{
    isInvincibleState = true;
    invincibilityTimer = SDL_GetTicks();
    invincibilityDuration = duration;
}

bool Player::isInvincible() const
{
    if(isInvincibleState && (SDL_GetTicks() - invincibilityTimer < invincibilityDuration))
    {
        return true;
    }
    return false;
}
