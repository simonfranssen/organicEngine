
# --------------------------------------------------
# Shader configuration
# --------------------------------------------------

set(SHADER_SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/Shaders/Source"
)

set(SHADER_OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/Shaders"
)

set(VERTEX_SHADER
    "${SHADER_SOURCE_DIR}/triangle.vert.hlsl"
)

set(FRAGMENT_SHADER
    "${SHADER_SOURCE_DIR}/triangle.frag.hlsl"
)


# --------------------------------------------------
# macOS - HLSL -> MSL -> AIR -> METALLIB
# --------------------------------------------------

if(APPLE)

    set(PLATFORM_SHADER_DIR
        "${SHADER_OUTPUT_DIR}/macOS"
    )

    set(VERTEX_MSL
        "${PLATFORM_SHADER_DIR}/triangle.vert.metal"
    )

    set(FRAGMENT_MSL
        "${PLATFORM_SHADER_DIR}/triangle.frag.metal"
    )

    set(VERTEX_AIR
        "${PLATFORM_SHADER_DIR}/triangle.vert.air"
    )

    set(FRAGMENT_AIR
        "${PLATFORM_SHADER_DIR}/triangle.frag.air"
    )

    set(METALLIB
        "${PLATFORM_SHADER_DIR}/OrganicEngine.metallib"
    )

    find_program(XCRUN_EXECUTABLE xcrun REQUIRED)

    add_custom_command(
        OUTPUT "${METALLIB}"

        BYPRODUCTS
            "${VERTEX_MSL}"
            "${FRAGMENT_MSL}"
            "${VERTEX_AIR}"
            "${FRAGMENT_AIR}"

        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PLATFORM_SHADER_DIR}"

        # HLSL -> MSL
        COMMAND $<TARGET_FILE:shadercross>
            "${VERTEX_SHADER}"
            --source HLSL
            --dest MSL
            --stage vertex
            --entrypoint vertexMain
            --output "${VERTEX_MSL}"

        COMMAND $<TARGET_FILE:shadercross>
            "${FRAGMENT_SHADER}"
            --source HLSL
            --dest MSL
            --stage fragment
            --entrypoint main
            --output "${FRAGMENT_MSL}"

        # MSL -> AIR
        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metal
            -c "${VERTEX_MSL}"
            -o "${VERTEX_AIR}"

        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metal
            -c "${FRAGMENT_MSL}"
            -o "${FRAGMENT_AIR}"

        # AIR -> METALLIB
        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metallib
            "${VERTEX_AIR}"
            "${FRAGMENT_AIR}"
            -o "${METALLIB}"

        DEPENDS
            shadercross
            "${VERTEX_SHADER}"
            "${FRAGMENT_SHADER}"

        COMMENT "Compiling OrganicEngine Metal shaders..."
        VERBATIM
    )

    add_custom_target(OrganicEngine_Shaders
        DEPENDS "${METALLIB}"
    )


# --------------------------------------------------
# Linux - HLSL -> SPIR-V
# --------------------------------------------------

elseif(UNIX)

    set(PLATFORM_SHADER_DIR
        "${SHADER_OUTPUT_DIR}/Linux"
    )

    set(VERTEX_SPIRV
        "${PLATFORM_SHADER_DIR}/triangle.vert.spv"
    )

    set(FRAGMENT_SPIRV
        "${PLATFORM_SHADER_DIR}/triangle.frag.spv"
    )

    add_custom_command(
        OUTPUT "${VERTEX_SPIRV}"

        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PLATFORM_SHADER_DIR}"

        COMMAND $<TARGET_FILE:shadercross>
            "${VERTEX_SHADER}"
            --source HLSL
            --dest SPIRV
            --stage vertex
            --entrypoint main
            --output "${VERTEX_SPIRV}"

        DEPENDS
            shadercross
            "${VERTEX_SHADER}"

        COMMENT "Compiling vertex shader to SPIR-V..."
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${FRAGMENT_SPIRV}"

        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PLATFORM_SHADER_DIR}"

        COMMAND $<TARGET_FILE:shadercross>
            "${FRAGMENT_SHADER}"
            --source HLSL
            --dest SPIRV
            --stage fragment
            --entrypoint main
            --output "${FRAGMENT_SPIRV}"

        DEPENDS
            shadercross
            "${FRAGMENT_SHADER}"

        COMMENT "Compiling fragment shader to SPIR-V..."
        VERBATIM
    )

    add_custom_target(OrganicEngine_Shaders
        DEPENDS
            "${VERTEX_SPIRV}"
            "${FRAGMENT_SPIRV}"
    )

else()

    message(FATAL_ERROR
        "OrganicEngine shader pipeline does not support this platform."
    )

endif()
