#include "CommonFunc.h"

bool is_solid(int tileID)
{
    return tileID == 1 || tileID == 2;
}

float getGroundLevel(float charX, float charWidth, int frameHeight)
{
    int col = static_cast<int>((charX + charWidth / 2) / TILE_SIZE);
    if(col < 0 || col >= MAP_WIDTH) return WINDOW_HEIGHT - frameHeight;

    for(int row = 0; row < MAP_HEIGHT; ++row)
    {
        if(is_solid(tileMap[row][col]))
        {
            return row * TILE_SIZE - frameHeight;
        }
    }
    return WINDOW_HEIGHT - frameHeight;
}
