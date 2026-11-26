#pragma once

#include <SDL3/SDL_gpu.h>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool Initialize(SDL_GPUDevice* device);

    bool Load(
        const char* filePath,
        SDL_GPUShaderStage stage,
        SDL_GPUShaderFormat format,
        const char* entrypoint
    );

    void Unload();
    SDL_GPUShader* Get() const;

private:
    SDL_GPUDevice* device = nullptr;
    SDL_GPUShader* shader = nullptr;
};

