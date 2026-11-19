#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "../Engine/Renderer/Renderer.h"

#include <iostream>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr<<"SDL Initialisation failed" << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OrganicEngine", 1280, 720, SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        std::cerr << "Window creation failed: "
                << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    std::cout<<"Window created succesfully!"<<std::endl;

        const SDL_GPUShaderFormat shaderFormats =
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_METALLIB;

    SDL_GPUDevice* gpuDevice =
        SDL_CreateGPUDevice(shaderFormats, true, nullptr);

    if (gpuDevice == nullptr)
    {
        std::cerr << "GPU device creation failed: "
                  << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "GPU driver: "
              << SDL_GetGPUDeviceDriver(gpuDevice) << '\n';

    // 4. Connect GPU device to window
    if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window))
    {
        std::cerr << "Window claim failed: "
                  << SDL_GetError() << '\n';

        SDL_DestroyGPUDevice(gpuDevice);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Renderer renderObj(gpuDevice,  window);

    if (!renderObj.Initialize())
    {
        std::cerr << "Render init failed: "
                  << SDL_GetError() << '\n';
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
        renderObj.Render();
    }


        // 9. Cleanup
    SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyGPUDevice(gpuDevice);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;


}