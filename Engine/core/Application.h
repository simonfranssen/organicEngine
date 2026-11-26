#pragma once

#include <SDL3/SDL.h>

#include "GameClock.h"
#include "../Renderer/Renderer.h"

class Application
{
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();
    bool CreateTestMesh();

private:
    SDL_Window* m_window = nullptr;
    SDL_GPUDevice* m_gpuDevice = nullptr;

    Renderer m_renderer;
    GameClock& m_clock = GameClock::Get();

    bool m_running = false;
    Mesh m_testMesh;


    bool InitSDL();
    bool CreateWindow();
    bool InitGPU();
    bool InitRenderer();

    void ProcessEvents();
    void Update();
    void Render();

};
