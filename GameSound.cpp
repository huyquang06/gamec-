#include "gamesound.h"
#include "CommonFunc.h"

GameSound::GameSound(const std::string& musicPath) : backgroundMusic(nullptr) {
    backgroundMusic = Mix_LoadMUS(musicPath.c_str());
    if (!backgroundMusic) {
        printf("Failed to load background music: %s\n", Mix_GetError());
    }
}

GameSound::~GameSound() {
    stopMusic();
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    Mix_CloseAudio();
    Mix_Quit();
}

bool GameSound::init() {
    // Không cần Mix_Init cho WAV vì SDL_mixer hỗ trợ WAV mặc định
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return false;
    }

    return true;
}

void GameSound::playMusic() {
    if (backgroundMusic && Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(backgroundMusic, -1);  // -1: lặp vô hạn
    }
}

void GameSound::stopMusic() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}
