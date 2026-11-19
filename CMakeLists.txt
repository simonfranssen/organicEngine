cmake_minimum_required(VERSION 3.25)

project(OrganicEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SDL3 configuration
include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3)

# Engine executable
add_executable(MyEngine
    src/main.cpp
    Engine/Renderer/Renderer.cpp
)

target_link_libraries(MyEngine PRIVATE SDL3::SDL3)

# Helpful compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(MyEngine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()