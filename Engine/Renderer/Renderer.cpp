#include "Renderer.h"
#include <SDL3/SDL.h>
#include "../Graphics/Vertex.h"
#include <cstddef>
#include "../Core/GameClock.h"

Renderer::~Renderer()
{
    if (m_graphicsPipeline != nullptr)
    {
        SDL_ReleaseGPUGraphicsPipeline(
            m_device,
            m_graphicsPipeline
        );

        m_graphicsPipeline = nullptr;
    }
}

bool Renderer::Initialize(SDL_GPUDevice* device, SDL_Window* window)
{
    if (device == nullptr || window == nullptr)
        return false;

    m_device = device;
    m_window = window;

    // 1. Initialize shaders with GPU device
    if (!m_vertexShader.Initialize(device))
        return false;

    if (!m_fragmentShader.Initialize(device))
        return false;

    // 2. Load vertex shader
    SDL_Log("Loading vertex shader...");

    if (!m_vertexShader.Load(
        ORGANIC_ENGINE_SHADER_PATH,
        SDL_GPU_SHADERSTAGE_VERTEX,
        SDL_GPU_SHADERFORMAT_METALLIB,
        "vertexMain"))
    {
        SDL_Log("Vertex shader failed!");
        return false;
    }

    SDL_Log("Vertex shader loaded!");

    // 3. Load fragment shader
    SDL_Log("Loading fragment shader...");

    if (!m_fragmentShader.Load(
        ORGANIC_ENGINE_SHADER_PATH,
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        SDL_GPU_SHADERFORMAT_METALLIB,
        "fragMain"))
    {
        SDL_Log("Fragment shader failed!");
        return false;
    }

    SDL_Log("Fragment shader loaded!");

    // 4. Create graphics pipeline
    if (!CreateGraphicsPipeline())
    {
        SDL_Log("Graphics pipeline creation failed!");
        return false;
    }

    SDL_Log("Graphics pipeline created!");

    return true;
}


void Renderer::Render(Mesh& mesh)
{

    SDL_GPUCommandBuffer* commandBuffer =
        SDL_AcquireGPUCommandBuffer(m_device);

    if (commandBuffer == nullptr)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to acquire GPU command buffer: %s",
            SDL_GetError()
        );
        return;
    }

    SDL_GPUTexture* swapchainTexture = nullptr;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            commandBuffer,
            m_window,
            &swapchainTexture,
            nullptr,
            nullptr))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Swapchain acquisition failed: %s",
            SDL_GetError()
        );

        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
    }

    if (swapchainTexture == nullptr)
    {
        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
    }

    SDL_GPUColorTargetInfo colorTarget{};

    colorTarget.texture = swapchainTexture;
    colorTarget.clear_color = {
        0.08f,
        0.15f,
        0.35f,
        1.0f
    };

    colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass =
        SDL_BeginGPURenderPass(
            commandBuffer,
            &colorTarget,
            1,
            nullptr
        );

    if (renderPass == nullptr)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to begin render pass: %s",
            SDL_GetError()
        );

        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;

    }

    SDL_BindGPUGraphicsPipeline(renderPass,m_graphicsPipeline);

    float time = static_cast<float>(GameClock::Get().GetTotalTime());

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &time, sizeof(time));

    // Get vertex buffer from mesh
    SDL_GPUBuffer* vertexBuffer = mesh.GetVertexBuffer();

    if (vertexBuffer == nullptr)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Mesh has no vertex buffer!"
        );

        SDL_EndGPURenderPass(renderPass);
        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
    }

    // Bind vertex buffer
    SDL_GPUBufferBinding vertexBufferBinding{};

    vertexBufferBinding.buffer = vertexBuffer;
    vertexBufferBinding.offset = 0;

    SDL_BindGPUVertexBuffers(
        renderPass,
        0,
        &vertexBufferBinding,
        1
    );


    SDL_DrawGPUPrimitives(
    renderPass,
    mesh.GetVertexCount(), 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "GPU submission failed: %s",
            SDL_GetError()
        );
        return;
    }
}


bool Renderer::CreateGraphicsPipeline()
{
    // Describe vertex buffer layout
    SDL_GPUVertexBufferDescription vertexBufferDescription{};

    vertexBufferDescription.slot = 0;
    vertexBufferDescription.pitch = sizeof(Vertex);
    vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDescription.instance_step_rate = 0;

    // Describe vertex attributes
    SDL_GPUVertexAttribute vertexAttributes[2]{};

    // Position
    vertexAttributes[0].location = 0;
    vertexAttributes[0].buffer_slot = 0;
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[0].offset = offsetof(Vertex, position);

    // Texture coordinates
    vertexAttributes[1].location = 1;
    vertexAttributes[1].buffer_slot = 0;
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    vertexAttributes[1].offset = offsetof(Vertex, texCoord);

    SDL_GPUVertexInputState vertexInputState{};

    vertexInputState.vertex_buffer_descriptions = &vertexBufferDescription;

    vertexInputState.num_vertex_buffers = 1;

    vertexInputState.vertex_attributes = vertexAttributes;

    vertexInputState.num_vertex_attributes = 2;

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.vertex_shader = m_vertexShader.Get();
    pipelineInfo.fragment_shader = m_fragmentShader.Get();

    pipelineInfo.vertex_input_state = vertexInputState;

    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    SDL_GPURasterizerState rasterizerState{};

    rasterizerState.fill_mode = SDL_GPU_FILLMODE_FILL;
    rasterizerState.cull_mode = SDL_GPU_CULLMODE_NONE;
    rasterizerState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    pipelineInfo.rasterizer_state = rasterizerState;

    SDL_GPUMultisampleState multisampleState{};

    multisampleState.sample_count = SDL_GPU_SAMPLECOUNT_1;

    pipelineInfo.multisample_state = multisampleState;

    SDL_GPUColorTargetDescription colorTargetDescription{};

    colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(m_device, m_window);

    SDL_GPUColorTargetDescription colorTargetDescriptions[] =
    {
        colorTargetDescription
    };

    pipelineInfo.target_info.color_target_descriptions =
        colorTargetDescriptions;

    pipelineInfo.target_info.num_color_targets = 1;

    m_graphicsPipeline = SDL_CreateGPUGraphicsPipeline(
    m_device,
    &pipelineInfo);

    if (m_graphicsPipeline == nullptr)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to create graphics pipeline: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;

}
