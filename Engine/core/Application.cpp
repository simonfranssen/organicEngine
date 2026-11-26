
#include "Application.h"

#include <iostream>

Application::Application() = default;

Application::~Application()
{
    Shutdown();
}

bool Application::Init()
{
    if (!InitSDL())
        return false;

    if (!CreateWindow())
        return false;

    if (!InitGPU())
        return false;

    if (!InitRenderer())
        return false;

    if (!CreateTestMesh())
        return false;

    m_running = true;

    return true;
}

bool Application::InitSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization failed: "
                  << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

bool Application::CreateWindow()
{
    m_window = SDL_CreateWindow(
        "OrganicEngine",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (m_window == nullptr)
    {
        std::cerr << "Window creation failed: "
                  << SDL_GetError() << '\n';
        return false;
    }

    std::cout << "Window created successfully!\n";

    return true;
}

bool Application::InitGPU()
{
    const SDL_GPUShaderFormat shaderFormats =
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_METALLIB;

    m_gpuDevice = SDL_CreateGPUDevice(
        shaderFormats,
        true,
        nullptr
    );

    if (m_gpuDevice == nullptr)
    {
        std::cerr << "GPU device creation failed: "
                  << SDL_GetError() << '\n';
        return false;
    }

    std::cout << "GPU driver: "
              << SDL_GetGPUDeviceDriver(m_gpuDevice)
              << '\n';

    if (!SDL_ClaimWindowForGPUDevice(m_gpuDevice, m_window))
    {
        std::cerr << "Window claim failed: "
                  << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

bool Application::InitRenderer()
{
    return m_renderer.Initialize(m_gpuDevice, m_window);
}

bool Application::CreateTestMesh()
{
    if (!m_testMesh.Initialize(m_gpuDevice))
        return false;

    Vertex vertices[] =
    {
        {{ 0.0f,  0.5f, 0.0f}, {0.5f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}}
    };

    return m_testMesh.Create(vertices, 3);
}

void Application::Run()
{
    while (m_running)
    {
        m_clock.Tick();

        ProcessEvents();
        Update();
        Render();
    }
}

void Application::ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            m_running = false;
    }
}

void Application::Update()
{
    double deltaTime = m_clock.GetDeltaTime();
    double totalTime = m_clock.GetTotalTime();
}

void Application::Render()
{
    m_renderer.Render(m_testMesh);
}

void Application::Shutdown()
{
    // Cleanup will be implemented next.
}
