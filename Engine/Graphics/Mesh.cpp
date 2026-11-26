
#include "Mesh.h"

#include <SDL3/SDL.h>
#include <cstring>


Mesh::~Mesh()
{
    Destroy();
}

bool Mesh::Create(const Vertex* vertices, Uint32 vertexCount)
{
    // Validate input
    if (device == nullptr || vertices == nullptr || vertexCount == 0)
    {
        SDL_Log("Invalid mesh creation parameters.");
        return false;
    }

    // Calculate vertex data size in bytes
    const Uint32 vertexDataSize =
        static_cast<Uint32>(sizeof(Vertex) * vertexCount);

    // Create GPU vertex buffer
    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bufferInfo.size = vertexDataSize;

    vertexBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);

    if (vertexBuffer == nullptr)
    {
        SDL_Log("Failed to create vertex buffer: %s", SDL_GetError());
        return false;
    }

    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = vertexDataSize;

    SDL_GPUTransferBuffer* transferBuffer =
        SDL_CreateGPUTransferBuffer(device, &transferInfo);

    if (transferBuffer == nullptr)
    {
        SDL_Log("Failed to create transfer buffer: %s", SDL_GetError());
        Destroy();
        return false;
    }

    // Map transfer buffer and copy vertex data
    void* mappedData =
        SDL_MapGPUTransferBuffer(device, transferBuffer, false);

    if (mappedData == nullptr)
    {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        Destroy();
        return false;
    }

    std::memcpy(mappedData, vertices, vertexDataSize);

    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    // Upload data to GPU
    SDL_GPUCommandBuffer* commandBuffer =
        SDL_AcquireGPUCommandBuffer(device);

    if (commandBuffer == nullptr)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        Destroy();
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation source{};
    source.transfer_buffer = transferBuffer;
    source.offset = 0;

    SDL_GPUBufferRegion destination{};
    destination.buffer = vertexBuffer;
    destination.offset = 0;
    destination.size = vertexDataSize;

    SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);

    SDL_EndGPUCopyPass(copyPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
    {
        SDL_Log("Failed to submit upload commands: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        Destroy();
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    this->vertexCount = vertexCount;

    return true;
}

void Mesh::Destroy()
{
    if (vertexBuffer != nullptr)
    {
        SDL_ReleaseGPUBuffer(device, vertexBuffer);
        vertexBuffer = nullptr;
    }

    vertexCount = 0;
}

SDL_GPUBuffer* Mesh::GetVertexBuffer() const
{
    return vertexBuffer;
}

Uint32 Mesh::GetVertexCount() const
{
    return vertexCount;
}

bool Mesh::Initialize(SDL_GPUDevice* gpuDevice)
{
    if (gpuDevice == nullptr)
    {
        SDL_Log("Cannot initialize mesh: GPU device is null.");
        return false;
    }

    device = gpuDevice;
    return true;
}