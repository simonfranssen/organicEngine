#include "Shader.h"
#include <SDL3/SDL.h>

Shader::~Shader()
{
    Unload();
}


void Shader::Unload()
{
    if (shader != nullptr)
    {
        SDL_ReleaseGPUShader(device, shader);
        shader = nullptr;
    }
}

bool Shader::Load(
    const char* filePath,
    SDL_GPUShaderStage stage,
    SDL_GPUShaderFormat format,
    const char* entrypoint)
{

    if (device == nullptr)
    {
        SDL_Log("Cannot load shader: GPU device is null.");
        return false;
    }

    Unload();

    size_t codeSize = 0;

    void* code = SDL_LoadFile(filePath, &codeSize);

    if (code == nullptr)
    {
        SDL_Log("Failed to load shader file: %s", SDL_GetError());
        return false;
    }

    SDL_GPUShaderCreateInfo createInfo{};

    //Vi formaterar all info i ett objekt och skickar som parameter istället
    createInfo.code = static_cast<const Uint8*>(code);
    createInfo.code_size = codeSize;
    createInfo.entrypoint = entrypoint;
    createInfo.format = format;
    createInfo.stage = stage;

    // Resource counts for our current shaders.
    createInfo.num_uniform_buffers = 1;

    if (stage == SDL_GPU_SHADERSTAGE_FRAGMENT)
    {
        createInfo.num_samplers = 1;
    }


    shader = SDL_CreateGPUShader(device, &createInfo);

    SDL_free(code);

    if (shader == nullptr)
    {
        SDL_Log("Failed to create GPU shader: %s", SDL_GetError());
        return false;
    }

    return true;

}

SDL_GPUShader* Shader::Get() const
{
    return shader;
}

bool Shader::Initialize(SDL_GPUDevice* gpuDevice)
{
    if (gpuDevice == nullptr)
    {
        SDL_Log("Cannot initialize shader: GPU device is null.");
        return false;
    }

    device = gpuDevice;
    return true;
}