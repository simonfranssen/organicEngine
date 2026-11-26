
cmake_minimum_required(VERSION 3.25)

project(OrganicEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ Configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Load build system modules
include(cmake/Dependencies.cmake)
include(cmake/CompilerOptions.cmake)

# Engine executable
add_executable(MyEngine
    src/main.cpp
    Engine/Renderer/Renderer.cpp
    Engine/Renderer/Shader.cpp
    Engine/Graphics/Mesh.cpp
    Engine/Core/Gameclock.cpp
    Engine/Core/Application.cpp
)

target_link_libraries(MyEngine PRIVATE
    SDL3::SDL3
)

organic_engine_set_compiler_options(MyEngine)

# Shader pipeline
include(cmake/Shaders.cmake)

# Compile shaders whenever the engine is built
add_dependencies(MyEngine OrganicEngine_Shaders)

target_compile_definitions(MyEngine PRIVATE
    ORGANIC_ENGINE_SHADER_PATH="${METALLIB}"
)