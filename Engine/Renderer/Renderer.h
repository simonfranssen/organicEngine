#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include "Shader.h"
#include "../Graphics/Mesh.h"



class Renderer
{
public:
   Renderer() = default;
   ~Renderer();

    //För att förbjuda anrop till konstruktorn efter 
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    bool CreateGraphicsPipeline();
    bool Initialize(SDL_GPUDevice* device, SDL_Window* window);
    void Render(Mesh& mesh);
    

private:

    SDL_GPUDevice* m_device = nullptr;
    SDL_Window* m_window = nullptr;
    Shader m_vertexShader;
    Shader m_fragmentShader;
    SDL_GPUGraphicsPipeline* m_graphicsPipeline = nullptr;
};