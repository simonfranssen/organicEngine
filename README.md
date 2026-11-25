# organicEngine

./build/_deps/sdl_shadercross-build/shadercross \
    Shaders/Source/triangle.frag.hlsl \
    --source HLSL \
    --dest MSL \
    --stage fragment \
    --entrypoint main \
    --output build/Shaders/triangle.frag.msl

./build/_deps/sdl_shadercross-build/shadercross \
    Shaders/Source/triangle.vert.hlsl \
    --source HLSL \
    --dest MSL \
    --stage vertex \
    --entrypoint main \
    --output build/Shaders/triangle.vert.msl