#include "CommonFunc.h"

bool is_solid(int tileID)
{
    return tileID == 1 || tileID == 3;
}

float getGroundLevel(float charX, float charWidth, int frameHeight, bool adjustForHeight)
{
    int col = static_cast<int>((charX + charWidth / 2) / TILE_SIZE);
    if (col < 0 || col >= MAP_WIDTH)
    {
        return adjustForHeight ? WINDOW_HEIGHT - frameHeight : WINDOW_HEIGHT;
    }

    for (int row = 0; row < MAP_HEIGHT; ++row)
    {
        if (is_solid(tileMap[row][col]))
        {
            float groundY = row * TILE_SIZE;
            return adjustForHeight ? groundY - frameHeight : groundY;
        }
    }
    return adjustForHeight ? WINDOW_HEIGHT - frameHeight : WINDOW_HEIGHT;
}
