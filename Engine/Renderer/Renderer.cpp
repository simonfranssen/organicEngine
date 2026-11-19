#include "Renderer.h"
#include <SDL3/SDL.h>

Renderer::Renderer(SDL_GPUDevice* device, SDL_Window* window)
    : device(device), window(window)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize()
{
    if (device == nullptr)
    {
        SDL_Log("Renderer initialization failed: GPU device is null.");
        return false;
    }

    if (window == nullptr)
    {
        SDL_Log("Renderer initialization failed: Window is null.");
        return false;
    }

    SDL_Log("Renderer initialized successfully!");
    return true;
}

void Renderer::Render()
{
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

    if (commandBuffer == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to acquire GPU command buffer: %s", SDL_GetError());
        return;
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
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Swapchain acquisition failed: %s", SDL_GetError());
        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
    }

        // Window may be minimized
    if (swapchainTexture == nullptr)
    {
        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
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

    if (renderPass == nullptr)
    {
    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to begin render pass: %s", SDL_GetError());
    SDL_CancelGPUCommandBuffer(commandBuffer);
    return;
    }

    // We have no draw calls yet.
    // The render pass only clears the screen.

    SDL_EndGPURenderPass(renderPass);

    // 8. Submit commands to GPU
    if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "GPU submission failed: %s", SDL_GetError());
        return;
    }
}