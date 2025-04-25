#ifndef COMMONFUNC_H_INCLUDED
#define COMMONFUNC_H_INCLUDED

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <fstream>
#include <sstream>
#include <cmath>

#define IDLE_FRAME_MONSTER 4

const int WALKING_FRAMES = 8;
const int IDLE_FRAME = 7;
const int ATTACK_FRAMES = 6;
const int WALKING_FRAME_DELAY = 100; // Milliseconds
const int IDLE_FRAME_DELAY = 100;    // Milliseconds
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 640;
const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS; // Thời gian mỗi khung hình (ms)

const int TILE_SIZE = 64;
const int MAP_WIDTH = 400;
const int MAP_HEIGHT = 10;
const int TILESET_COLS = 2;

const float COLLISION_MARGIN = 0.0f;

float getGroundLevel(float charX, float charWidth, int frameHeight, bool adjustForHeight = true);
bool is_solid(int tileID);
extern int tileMap[MAP_HEIGHT][MAP_WIDTH];

#endif // COMMONFUNC_H_INCLUDED
