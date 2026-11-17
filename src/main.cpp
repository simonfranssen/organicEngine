#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

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


    bool running = true;
    SDL_Event event;

    while (running)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // Acquire command buffer
        SDL_GPUCommandBuffer* commandBuffer =
            SDL_AcquireGPUCommandBuffer(gpuDevice);

        if (commandBuffer == nullptr)
        {
            std::cerr << "Command buffer acquisition failed: "
                      << SDL_GetError() << '\n';
            break;
        }

        // Acquire swapchain texture
        SDL_GPUTexture* swapchainTexture = nullptr;

        if (!SDL_WaitAndAcquireGPUSwapchainTexture(
                commandBuffer,
                window,
                &swapchainTexture,
                nullptr,
                nullptr))
        {
            std::cerr << "Swapchain acquisition failed: "
                      << SDL_GetError() << '\n';

            SDL_CancelGPUCommandBuffer(commandBuffer);
            break;
        }

        // Window may be minimized
        if (swapchainTexture == nullptr)
        {
            SDL_CancelGPUCommandBuffer(commandBuffer);
            continue;
        }
        // 6. Configure color target
        SDL_GPUColorTargetInfo colorTarget = {};

        colorTarget.texture = swapchainTexture;

        colorTarget.clear_color = {
            0.08f,  // Red
            0.15f,  // Green
            0.35f,  // Blue
            1.0f    // Alpha
        };

        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        // 7. Begin render pass
        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(
                commandBuffer,
                &colorTarget,
                1,
                nullptr
            );

        // We have no draw calls yet.
        // The render pass only clears the screen.

        SDL_EndGPURenderPass(renderPass);

        // 8. Submit commands to GPU
        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
        {
            std::cerr << "GPU submission failed: "
                      << SDL_GetError() << '\n';
            break;
        }
    }


        // 9. Cleanup
    SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyGPUDevice(gpuDevice);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;


}