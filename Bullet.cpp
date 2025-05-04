#include "Bullet.h"

Bullet::Bullet(SDL_Renderer* renderer, float x, float y, float velocityX) : x(x), y(y), velocityX(velocityX)
{
    texture = IMG_LoadTexture(renderer, "image/BULLET_RIGHT.png");
    if (!texture)
    {
        SDL_Log("Failed to load bullet texture: %s", IMG_GetError());
        width = 0;
        height = 0;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }
}

void Bullet::Update(const int map[MAP_HEIGHT][MAP_WIDTH])
{
    x += velocityX;

    int row = static_cast<int>(y / TILE_SIZE);
    int col = static_cast<int>(x / TILE_SIZE);

    if (row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH)
    {
        if (map[row][col] == 1 || map[row][col] == 2)
        {
            x = -1000; // Đánh dấu để xóa Bullet khi va chạm với tường
        }
    }
}

void Bullet::Render(SDL_Renderer* renderer, SDL_Rect& camera)
{
    SDL_Rect dstRect = {(int)(x - camera.x), (int)(y - camera.y), width, height};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

SDL_Rect Bullet::GetBoundingBox() const
{
    const int offsetX = 5;
    const int offsetY = 5;
    const int shrinkW = 10;
    const int shrinkH = 10;
    return {(int)x + offsetX, (int)y + offsetY, width - shrinkW, height - shrinkH};
}

bool Bullet::isOutOfBounds() const
{
    return x < 0 || x > MAP_WIDTH * TILE_SIZE;
}
