#include "GameClock.h"

GameClock& GameClock::Get()
{
    static GameClock instance;
    return instance;
}

void GameClock::Tick()
{
    Uint64 currentCounter = SDL_GetPerformanceCounter();

    m_deltaTime =
        static_cast<double>(currentCounter - m_previousCounter) /
        static_cast<double>(SDL_GetPerformanceFrequency());

    m_previousCounter = currentCounter;
    m_totalTime += m_deltaTime;
}

double GameClock::GetDeltaTime() const
{
    return m_deltaTime;
}

double GameClock::GetTotalTime() const
{
    return m_totalTime;
}

GameClock::GameClock()
{
    m_previousCounter = SDL_GetPerformanceCounter();
}