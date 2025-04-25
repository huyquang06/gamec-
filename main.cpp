#include "CommonFunc.h"
#include "Monster.h"
#include "Player.h"
#include <vector>

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

int main(int argc, char* argv[]) {
    LoadTile("map/mapgame.dat", tileMap);

    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL/IMG init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load background and tile textures
    SDL_Texture* layer1 = IMG_LoadTexture(renderer, "image/background_layer_1.png");
    SDL_Texture* layer2 = IMG_LoadTexture(renderer, "image/background_layer_2.png");
    SDL_Texture* layer3 = IMG_LoadTexture(renderer, "image/background_layer_3.png");
    SDL_Texture* block1 = IMG_LoadTexture(renderer, "map/BLOCK_1.png");
    SDL_Texture* block2 = IMG_LoadTexture(renderer, "map/BLOCK_2.png");

    // Initialize monsters
    SDL_Texture* m_idleTexture = IMG_LoadTexture(renderer, "image/IDLE_MONSTER.png");
    SDL_Texture* m_attackTexture = IMG_LoadTexture(renderer, "image/ATTACK.png");
    int m_idleWidth, m_idleHeight;
    SDL_QueryTexture(m_idleTexture, NULL, NULL, &m_idleWidth, &m_idleHeight);
    int m_width = m_idleWidth / 4;
    int m_height = m_idleHeight;
    std::vector<SDL_FPoint> m_positions = Generate_Monsters(25, 300, 900, MAP_WIDTH * TILE_SIZE, m_width, m_height);
    std::vector<Monster> monsters = InitMonsters(renderer, "image/IDLE_MONSTER.png", "image/ATTACK.png", 4, 7, m_positions);

    // Initialize player
    int textureWidth, textureHeight;
    SDL_Texture* walkRTexture = IMG_LoadTexture(renderer, "image/RUN.png");
    SDL_QueryTexture(walkRTexture, NULL, NULL, &textureWidth, &textureHeight);
    int frameWidth = textureWidth / WALKING_FRAMES;
    int frameHeight = textureHeight;
    Player player(renderer, 0.0f, getGroundLevel(0, frameWidth, frameHeight, true));

    SDL_Rect camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    bool running = true;
    SDL_Event event;

    while (running && !player.isGameOver()) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            player.handleInput(event);
        }

        player.Update(camera, monsters);

        for(auto& monster : monsters)
        {
            monster.Update(player);
        }

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

        // Render player and monsters
        player.Render(renderer, camera);
        RenderMonsters(renderer, monsters, camera);

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - frameTime);
    }

    // Cleanup
    SDL_DestroyTexture(layer1);
    SDL_DestroyTexture(layer2);
    SDL_DestroyTexture(layer3);
    SDL_DestroyTexture(block1);
    SDL_DestroyTexture(block2);
    SDL_DestroyTexture(walkRTexture);
    SDL_DestroyTexture(m_idleTexture);
    SDL_DestroyTexture(m_attackTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
