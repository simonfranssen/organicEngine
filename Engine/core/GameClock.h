#pragma once

#include <SDL3/SDL.h>

class GameClock
{
public:
    static GameClock& Get();

    void Tick();

    double GetDeltaTime() const;

    double GetTotalTime() const;
 
private:
    GameClock();
    GameClock(const GameClock&) = delete;
    
    GameClock& operator=(const GameClock&) = delete;

    Uint64 m_previousCounter = 0;

    double m_deltaTime = 0.0;
    double m_totalTime = 0.0;
};