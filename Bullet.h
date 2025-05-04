#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include "CommonFunc.h"

class Bullet
{
public:
    Bullet(SDL_Renderer* renderer, float x, float y, float velocityX);
    void Update(const int map[MAP_HEIGHT][MAP_WIDTH]);
    void Render(SDL_Renderer* renderer, SDL_Rect& camera);
    SDL_Rect GetBoundingBox() const;
    bool isOutOfBounds() const;

    float getX() const { return x; } // Getter cho x
    float getY() const { return y; } // Getter cho y

private:
    SDL_Texture* texture;
    float x, y;
    float velocityX;
    int width, height;
};

#endif // BULLET_H_INCLUDED
