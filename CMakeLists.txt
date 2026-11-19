
cmake_minimum_required(VERSION 3.25)

project(OrganicEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

# --------------------------------------------------
# C++ Configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# --------------------------------------------------
# Dependencies
# --------------------------------------------------

include(FetchContent)

# SDL3 configuration
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


# SDL_shadercross configuration
set(SDLSHADERCROSS_VENDORED ON CACHE BOOL "" FORCE)
set(SDLSHADERCROSS_DXC ON CACHE BOOL "" FORCE)
set(SDLSHADERCROSS_CLI ON CACHE BOOL "" FORCE)
set(SDLSHADERCROSS_TESTS OFF CACHE BOOL "" FORCE)
set(SDLSHADERCROSS_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL_shadercross
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_shadercross.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE TRUE
)

if(TARGET SDL_shadercross)
    message(STATUS "SDL_shadercross library found")
endif()

if(TARGET shadercross)
    message(STATUS "shadercross CLI target found")
endif()

FetchContent_MakeAvailable(SDL_shadercross)


# --------------------------------------------------
# Engine
# --------------------------------------------------

add_executable(MyEngine
    src/main.cpp
    Engine/Renderer/Renderer.cpp
)

target_link_libraries(MyEngine PRIVATE
    SDL3::SDL3
)


# --------------------------------------------------
# Compiler Warnings
# --------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(MyEngine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

