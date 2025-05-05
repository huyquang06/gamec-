#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "CommonFunc.h"

class Timer {
public:
    Timer();
    void start();
    void stop();
    void pause();
    void unpause();
    Uint32 getTicks() const;
    bool isStarted() const;
    bool isPaused() const;

private:
    Uint32 startTicks;
    Uint32 pausedTicks;
    bool started;
    bool paused;
};

#endif // TIMER_H_INCLUDED
