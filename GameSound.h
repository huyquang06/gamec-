#ifndef GAMESOUND_H_INCLUDED
#define GAMESOUND_H_INCLUDED

#include "CommonFunc.h"
class GameSound {
public:
    GameSound(const std::string& musicPath);
    ~GameSound();
    bool init();
    void playMusic();
    void stopMusic();

private:
    Mix_Music* backgroundMusic;
};


#endif // GAMESOUND_H_INCLUDED
