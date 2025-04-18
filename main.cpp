#include "CommonFunc.h"
#include <cmath>

enum CharacterState {
    IDLE_RIGHT, IDLE_LEFT, WALKING_RIGHT, WALKING_LEFT
};

int tileMap[MAP_HEIGHT][MAP_WIDTH];

void LoadTile(const char* filename, int map[MAP_HEIGHT][MAP_WIDTH]) {
    std::ifstream file(filename);
    std::string line;
    int row = 0;
    while (std::getline(file, line) && row < MAP_HEIGHT) {
        std::stringstream ss(line);
        std::string token;
        int col = 0;
        while (ss >> token && col < MAP_WIDTH) {
            map[row][col] = std::stoi(token);
            col++;
        }
        row++;
    }
    file.close();
}

bool is_solid(int tile) {
    return tile == 1 || tile == 2;
}

float getGroundLevel(float charX, float charWidth, int frameHeight) {
    int col = static_cast<int>((charX + charWidth / 2) / TILE_SIZE);
    if (col < 0 || col >= MAP_WIDTH) return WINDOW_HEIGHT - frameHeight;

    for (int row = 0; row < MAP_HEIGHT; ++row) {
        if (is_solid(tileMap[row][col])) {
            return row * TILE_SIZE - frameHeight;
        }
    }
    return WINDOW_HEIGHT - frameHeight;
}

bool checkCollision_x(float new_x, float y, int frameWidth, int frameHeight) {
    int left = static_cast<int>(new_x) / TILE_SIZE;
    int right = static_cast<int>(new_x + frameWidth - 1) / TILE_SIZE;
    int top = static_cast<int>(y) / TILE_SIZE;
    int bottom = static_cast<int>(y + frameHeight - 1) / TILE_SIZE;

    for (int row = top; row <= bottom; row++) {
        if ((left >= 0 && left < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT && is_solid(tileMap[row][left])) ||
            (right >= 0 && right < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT && is_solid(tileMap[row][right])))
            return true;
    }
    return false;
}

bool checkCollision_y(float x, float new_y, int frameWidth, int frameHeight) {
    int left = static_cast<int>(x) / TILE_SIZE;
    int right = static_cast<int>(x + frameWidth - 1) / TILE_SIZE;
    int top = static_cast<int>(new_y) / TILE_SIZE;
    int bottom = static_cast<int>(new_y + frameHeight - 1) / TILE_SIZE;

    for (int col = left; col <= right; col++) {
        if ((top >= 0 && top < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH && is_solid(tileMap[top][col])) ||
            (bottom >= 0 && bottom < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH && is_solid(tileMap[bottom][col])))
            return true;
    }
    return false;
}

float find_RespawnPoint(float fall_x)
{
    int col = static_cast<int>(fall_x / TILE_SIZE);

    while(col >= 0)
    {
        for(int row = 0; row < MAP_HEIGHT - 1; row++)
        {
            if(!is_solid(tileMap[row][col]) && is_solid(tileMap[row + 1][col]))
            {
                return col * TILE_SIZE;
            }
        }
        col--;
    }
    return 0.0f;
}

int main(int argc, char* argv[]) {
    LoadTile("map/mapgame.dat", tileMap);

    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL/IMG init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load textures
    SDL_Texture* layer1 = IMG_LoadTexture(renderer, "image/background_layer_1.png");
    SDL_Texture* layer2 = IMG_LoadTexture(renderer, "image/background_layer_2.png");
    SDL_Texture* layer3 = IMG_LoadTexture(renderer, "image/background_layer_3.png");
    SDL_Texture* block1 = IMG_LoadTexture(renderer, "map/BLOCK_1.png");
    SDL_Texture* block2 = IMG_LoadTexture(renderer, "map/BLOCK_2.png");
    SDL_Texture* walkRTexture = IMG_LoadTexture(renderer, "image/RUN.png");
    SDL_Texture* walkLTexture = IMG_LoadTexture(renderer, "image/RUN_NGUOC.png");
    SDL_Texture* idleLTexture = IMG_LoadTexture(renderer, "image/IDLE_LEFT.png");
    SDL_Texture* idleRTexture = IMG_LoadTexture(renderer, "image/IDLE.png");

    int textureWidth, textureHeight;
    SDL_QueryTexture(walkRTexture, NULL, NULL, &textureWidth, &textureHeight);
    int frameWidth = textureWidth / WALKING_FRAMES;
    int frameHeight = textureHeight;

    int idleTextureWidth, idleTextureHeight;
    SDL_QueryTexture(idleRTexture, NULL, NULL, &idleTextureWidth, &idleTextureHeight);
    int idleFrameWidth = idleTextureWidth / IDLE_FRAME;
    int idleFrameHeight = idleTextureHeight;

    float x = 0.0f, y = getGroundLevel(0, frameWidth, frameHeight);

    bool moveLeft = false, moveRight = false, facingRight = true, isJumping = false;
    int walkingFrame = 0, idleFrame = 0;
    Uint32 frameTimer = SDL_GetTicks();

    SDL_Rect camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    CharacterState currentState = IDLE_RIGHT;

    bool running = true;
    SDL_Event event;

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a) { moveLeft = true; facingRight = false; }
                if (event.key.keysym.sym == SDLK_d) { moveRight = true; facingRight = true; }
                if (event.key.keysym.sym == SDLK_SPACE && !isJumping) {
                    isJumping = true;
                    velocity = -jumpForce;
                }
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_a) moveLeft = false;
                if (event.key.keysym.sym == SDLK_d) moveRight = false;
            }
        }

        if (moveLeft) {
            new_x = x - speed;
            if (!checkCollision_x(new_x, y, frameWidth, frameHeight)) x = new_x;
        }
        if (moveRight) {
            new_x = x + speed;
            if (!checkCollision_x(new_x, y, frameWidth, frameHeight)) x = new_x;
        }

        if (x < 0) x = 0;
        if (x > MAP_WIDTH * TILE_SIZE - frameWidth) x = MAP_WIDTH * TILE_SIZE - frameWidth;

        // Gravity
        velocity += gravity;
        float new_y = y + velocity;

        bool fall_down = new_y > WINDOW_HEIGHT;

        if(fall_down)
        {
            x = find_RespawnPoint(x);
            y = getGroundLevel(x, frameWidth, frameHeight);

            velocity = 0;
            isJumping = false;
        }
        else if (!checkCollision_y(x, new_y, frameWidth, frameHeight))
        {
            y = new_y;
        }
        else
        {
            if (velocity > 0)
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

        camera.x = (int)x + frameWidth / 2 - WINDOW_WIDTH / 2;
        camera.y = 0;
        if (camera.x < 0) camera.x = 0;
        if (camera.x > MAP_WIDTH * TILE_SIZE - WINDOW_WIDTH) camera.x = MAP_WIDTH * TILE_SIZE - WINDOW_WIDTH;

        currentState = moveLeft ? WALKING_LEFT :
                       moveRight ? WALKING_RIGHT :
                       facingRight ? IDLE_RIGHT : IDLE_LEFT;

        if (SDL_GetTicks() > frameTimer) {
            if (currentState == WALKING_LEFT || currentState == WALKING_RIGHT) {
                walkingFrame = (walkingFrame + 1) % WALKING_FRAMES;
                frameTimer = SDL_GetTicks() + WALKING_FRAME_DELAY;
            } else {
                idleFrame = (idleFrame + 1) % IDLE_FRAME;
                frameTimer = SDL_GetTicks() + IDLE_FRAME_DELAY;
            }
        }

        SDL_Rect spriteClip = {
            .x = (currentState == WALKING_LEFT || currentState == WALKING_RIGHT) ? walkingFrame * frameWidth : idleFrame * idleFrameWidth,
            .y = 0,
            .w = (currentState == WALKING_LEFT || currentState == WALKING_RIGHT) ? frameWidth : idleFrameWidth,
            .h = (currentState == WALKING_LEFT || currentState == WALKING_RIGHT) ? frameHeight : idleFrameHeight
        };

        SDL_Rect charRect = {(int)x - camera.x, (int)y - camera.y, spriteClip.w, spriteClip.h};

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw backgrounds (parallax)
        int bg1_width, bg1_height, bg2_width, bg2_height, bg3_width, bg3_height;
        SDL_QueryTexture(layer1, NULL, NULL, &bg1_width, &bg1_height);
        SDL_QueryTexture(layer2, NULL, NULL, &bg2_width, &bg2_height);
        SDL_QueryTexture(layer3, NULL, NULL, &bg3_width, &bg3_height);

        for (int i = -camera.x / 1.5; i < WINDOW_WIDTH + bg1_width; i += bg1_width) {
    SDL_Rect dst = {(int)i, 0, bg1_width, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, layer1, NULL, &dst);
}

for (int i = -camera.x / 1.2; i < WINDOW_WIDTH + bg2_width; i += bg2_width) {
    SDL_Rect dst = {(int)i, 0, bg2_width, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, layer2, NULL, &dst);
}

for (int i = -camera.x; i < WINDOW_WIDTH + bg3_width; i += bg3_width) {
    SDL_Rect dst = {(int)i, 0, bg3_width, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, layer3, NULL, &dst);
}

        // Draw tile map
        for (int row = 0; row < MAP_HEIGHT; row++) {
            for (int col = 0; col < MAP_WIDTH; col++) {
                int tile = tileMap[row][col];
                if (tile == 0) continue;
                SDL_Texture* tex = (tile == 1) ? block1 : block2;
                SDL_Rect dst = {col * TILE_SIZE - camera.x, row * TILE_SIZE - camera.y, TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, tex, NULL, &dst);
            }
        }

        SDL_Texture* currentTexture = (currentState == WALKING_LEFT) ? walkLTexture :
                                      (currentState == WALKING_RIGHT) ? walkRTexture :
                                      (currentState == IDLE_LEFT) ? idleLTexture : idleRTexture;

        SDL_RenderCopy(renderer, currentTexture, &spriteClip, &charRect);
        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - frameTime);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
