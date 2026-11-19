#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

class Renderer
{
public:
    Renderer(SDL_GPUDevice* device, SDL_Window* SDL_Window);
    ~Renderer();

    //För att förbjuda anrop till konstruktorn efter 
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Initialize();
    void Render();

private:
    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;
};