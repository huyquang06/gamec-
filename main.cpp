#include "CommonFunc.h"
#include "Monster.h"
#include "Player.h"
#include "Timer.h"
#include "GameSound.h"

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

    if(TTF_Init() == -1)
    {
        printf("SDL_ttf init failed: %s\n", TTF_GetError());
        return 1;
    }

    // gamesound
    GameSound background_music ("sound/background_sound.mp3");
    background_music.playMusic();



    SDL_Window* window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("font/pixel_font.otf", 18);

    // load game button
    SDL_Surface* pauseSurface = IMG_Load("image/pause_button.png");
    SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface);

    int pauseWidth, pauseHeight;
    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseWidth, &pauseHeight);
    SDL_Rect pauseRect = {WINDOW_WIDTH - pauseWidth - 10, 10, pauseWidth, pauseHeight};

    // load game over
    SDL_Texture* gameOverTexture = IMG_LoadTexture(renderer, "image/game_over.png");
    int gameOverWidth, gameOverHeight;
    SDL_QueryTexture(gameOverTexture, NULL, NULL, &gameOverWidth, &gameOverHeight);
    SDL_Rect gameOverRect = {(WINDOW_WIDTH - gameOverWidth) / 2, (WINDOW_HEIGHT - gameOverHeight) / 2, gameOverWidth, gameOverHeight};

    // load start button
    SDL_Texture* startTexture = IMG_LoadTexture(renderer,"image/game_start.png");

    int startWidth, startHeight;
    SDL_QueryTexture(startTexture, NULL, NULL, &startWidth, &startHeight);
    SDL_Rect startRect = {(WINDOW_WIDTH - startWidth) / 2, (WINDOW_HEIGHT - startHeight) / 2, startWidth, startHeight};

    // Load background and tile textures
    SDL_Texture* layer1 = IMG_LoadTexture(renderer, "image/background_layer_1.png");
    SDL_Texture* layer2 = IMG_LoadTexture(renderer, "image/background_layer_2.png");
    SDL_Texture* layer3 = IMG_LoadTexture(renderer, "image/background_layer_3.png");
    SDL_Texture* block1 = IMG_LoadTexture(renderer, "map/BLOCK_1.png");
    SDL_Texture* block2 = IMG_LoadTexture(renderer, "map/BLOCK_3.png");
    SDL_Texture* heartTexture = IMG_LoadTexture(renderer, "image/HEART.png");

    int heartWidth, heartHeight;
    SDL_QueryTexture(heartTexture, NULL, NULL, &heartWidth, &heartHeight);

    // monsters
    SDL_Texture* m_idleTexture = IMG_LoadTexture(renderer, "image/IDLE_MONSTER.png");
    SDL_Texture* m_attackTexture = IMG_LoadTexture(renderer, "image/ATTACK_RIGHT.png");
    int m_idleWidth, m_idleHeight;
    SDL_QueryTexture(m_idleTexture, NULL, NULL, &m_idleWidth, &m_idleHeight);
    int m_width = m_idleWidth / 4;
    int m_height = m_idleHeight;
    std::vector<SDL_FPoint> m_positions = Generate_Monsters(25, 300, 900, MAP_WIDTH * TILE_SIZE, m_width, m_height, tileMap);
    std::vector<Monster> monsters = InitMonsters(renderer, "image/IDLE_MONSTER_RIGHT.png", "image/IDLE_MONSTER.png", "image/ATTACK_RIGHT.png", "image/ATTACK.png", 4, 7, m_positions, tileMap);

    // player
    int textureWidth, textureHeight;
    SDL_Texture* walkRTexture = IMG_LoadTexture(renderer, "image/RUN.png");
    SDL_QueryTexture(walkRTexture, NULL, NULL, &textureWidth, &textureHeight);
    int frameWidth = textureWidth / WALKING_FRAMES;
    int frameHeight = textureHeight;
    Player player(renderer, 0.0f, getGroundLevel(0, frameWidth, frameHeight, true, tileMap));

    int monstersDefeated = 0;

    SDL_Rect camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    bool running = true;
    bool isPaused = false;
    bool isStarted = false;
    bool gameOver = false;
    SDL_Event event;

    Timer gameTimer;

    SDL_Color whiteColor = {255, 255, 255, 255};
    SDL_Color continueColor = {50, 255, 50, 255}; // Màu xanh lá
    SDL_Color exitColor = {255, 50, 50, 255};     // Màu đỏ

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                if (!isStarted && mouseX >= startRect.x && mouseX <= startRect.x + startRect.w &&
                    mouseY >= startRect.y && mouseY <= startRect.y + startRect.h) {
                    isStarted = true;
                    gameTimer.start();
                }
                if (isStarted && mouseX >= pauseRect.x && mouseX <= pauseRect.x + pauseRect.w &&
                    mouseY >= pauseRect.y && mouseY <= pauseRect.y + pauseRect.h) {
                    isPaused = true;
                    gameTimer.pause();

                }
            }

            if (isStarted && isPaused && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_y) {
                    isPaused = false;
                    gameTimer.unpause();
                }
                else if (event.key.keysym.sym == SDLK_n) {
                    running = false;
                }
            }

             if (gameOver) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                }
            }
            if (isStarted && !isPaused && !gameOver) {
                player.handleInput(event);
            }
        }

        if (isStarted && !gameOver)
        {
            if (!isPaused) {
                player.Update(camera, monsters);

                for (auto& monster : monsters) {
                    monster.Update(player);
                }

                size_t initialSize = monsters.size();
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](const Monster& monster) { return monster.isMarkedForDeletion(); }),
                               monsters.end());
                monstersDefeated += initialSize - monsters.size();
            }
            if(player.isGameOver())
            {
                gameOver = true;
                gameTimer.pause();
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if(isStarted && !gameOver)
        {

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

        // tile map
        for (int row = 0; row < MAP_HEIGHT; row++) {
            for (int col = 0; col < MAP_WIDTH; col++) {
                int tile = tileMap[row][col];
                if (tile == 0) continue;
                SDL_Texture* tex = (tile == 1) ? block1 : block2;
                SDL_Rect dst = {col * TILE_SIZE - camera.x, row * TILE_SIZE - camera.y, TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, tex, NULL, &dst);
            }
        }

        // Render player va monsters
        player.Render(renderer, camera);
        RenderMonsters(renderer, monsters, camera);

        // render hearts
        int maxLives = 3;
        int remainingLives = maxLives - player.getRespawnCount();
        for(int i = 0; i < remainingLives; i++)
        {
            SDL_Rect heartDst = { i * (heartWidth - 5), 10, heartWidth, heartHeight};
            SDL_RenderCopy(renderer, heartTexture, NULL, &heartDst);
        }

        // bo dem diem
        SDL_Color textColor = {255,255,255,255};
        std::string pointText = "POINTS: " + std::to_string(monstersDefeated);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, pointText.c_str(), textColor);
        if (!textSurface)
        {
            printf("Failed to render text surface: %s\n", TTF_GetError());
        }
        else
        {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (!textTexture) {
                printf("Failed to create text texture: %s\n", SDL_GetError());
            } else {
                int textWidth = textSurface->w;
                int textHeight = textSurface->h;
                SDL_Rect textDst = {20 , heartHeight, textWidth, textHeight};
                SDL_RenderCopy(renderer, textTexture, NULL, &textDst);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }

        // hien thi seconds
        Uint32 timeElapsed = gameTimer.getTicks();
        Uint32 seconds = timeElapsed / 1000;  // millisecond to second
        std::stringstream timeStream;
        timeStream << "SECONDS: " << seconds << "s";
        std::string timeText = timeStream.str();
        SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor);
        if (timeSurface) {
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            if (timeTexture) {
                int timeWidth = timeSurface->w;
                int timeHeight = timeSurface->h;
                SDL_Rect timeDst = {20, 10 + heartHeight + timeHeight, timeWidth, timeHeight};
                SDL_RenderCopy(renderer, timeTexture, NULL, &timeDst);
                SDL_DestroyTexture(timeTexture);
            }
            SDL_FreeSurface(timeSurface);
        }
        if(!isPaused)
        {
            SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseRect);
        }

        if(isPaused)
        {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect fullScreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderFillRect(renderer, &fullScreen);

            // press y
            SDL_Surface* continueSurface = TTF_RenderText_Solid(font, "PRESS Y TO CONTINUE", continueColor);
            int continueWidth = 0, continueHeight = 0;
            SDL_Texture* continueTexture = NULL;

            if(continueSurface)
            {
                continueTexture = SDL_CreateTextureFromSurface(renderer, continueSurface);
                continueWidth = continueSurface->w;
                continueHeight = continueSurface->h;
                SDL_FreeSurface(continueSurface);
            }

            SDL_Surface* endSurface = TTF_RenderText_Solid(font, "PRESS N TO EXIT GAME", exitColor);
            int endWidth = 0, endHeight = 0;
            SDL_Texture* endTexture = NULL;

            if(endSurface)
            {
                endTexture = SDL_CreateTextureFromSurface(renderer, endSurface);
                endWidth = endSurface->w;
                endHeight = endSurface->h;
                SDL_FreeSurface(endSurface);
            }

            int totalHeight = continueHeight + endHeight + 20;
            int startY = (WINDOW_HEIGHT - totalHeight) / 2;

            if(continueTexture)
            {
                SDL_Rect continueRect = {(WINDOW_WIDTH - continueWidth) / 2, startY, continueWidth, continueHeight};
                SDL_RenderCopy(renderer, continueTexture, NULL, &continueRect);
                SDL_DestroyTexture(continueTexture);
            }

            if(endTexture)
            {
                SDL_Rect endRect = {(WINDOW_WIDTH - endWidth) / 2, startY + continueHeight + 20, endWidth, endHeight};
                SDL_RenderCopy(renderer, endTexture, NULL, &endRect);
                SDL_DestroyTexture(endTexture);
            }
        }

        }
        else if(gameOver)
        {
            SDL_SetRenderDrawColor(renderer,123,123,123,255);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);

            SDL_Color exitColor = {255,53,53,255};
            SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "PRESS ESC TO EXIT GAME", exitColor);
            if(exitSurface)
            {
                SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
                if(exitTexture)
                {
                    int exitWidth = exitSurface->w;
                    int exitHeight = exitSurface->h;
                    SDL_Rect exitRect = {(WINDOW_WIDTH - exitWidth) / 2, gameOverRect.y + gameOverRect.h + 20, exitWidth, exitHeight};
                    SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);
                    SDL_DestroyTexture(exitTexture);
                }
                SDL_FreeSurface(exitSurface);
            }
        }

        else
        {
            SDL_SetRenderDrawColor(renderer, 255,255,255,255);
            SDL_RenderClear(renderer);

            SDL_Color titleColor = {255,0,0,255};
            SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "HELL KILLER", titleColor);
             if (titleSurface) {
                SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
                if (titleTexture) {
                    int titleWidth = titleSurface->w;
                    int titleHeight = titleSurface->h;
                    SDL_Rect titleRect = {(WINDOW_WIDTH - titleWidth) / 2, (WINDOW_HEIGHT - startHeight) / 2 - titleHeight - 20, titleWidth, titleHeight};
                    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
                    SDL_DestroyTexture(titleTexture);
                }
                SDL_FreeSurface(titleSurface);
        }

        SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
        }

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - frameTime);
    }

    // Cleanup
    TTF_CloseFont(font);
    TTF_Quit();
    Mix_Quit();
    background_music.stopMusic();
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(pauseTexture);
    SDL_DestroyTexture(layer1);
    SDL_DestroyTexture(layer2);
    SDL_DestroyTexture(layer3);
    SDL_DestroyTexture(block1);
    SDL_DestroyTexture(block2);
    SDL_DestroyTexture(walkRTexture);
    SDL_DestroyTexture(m_idleTexture);
    SDL_DestroyTexture(m_attackTexture);
    SDL_DestroyTexture(heartTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
