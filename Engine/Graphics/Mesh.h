
#pragma once

#include <SDL3/SDL_gpu.h>
#include "Vertex.h"

class Mesh
{
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool Initialize(SDL_GPUDevice* device);

    bool Create(const Vertex* vertices, Uint32 vertexCount);
    void Destroy();

    SDL_GPUBuffer* GetVertexBuffer() const;
    Uint32 GetVertexCount() const;

private:
    SDL_GPUDevice* device = nullptr;
    SDL_GPUBuffer* vertexBuffer = nullptr;

    Uint32 vertexCount = 0;
};
