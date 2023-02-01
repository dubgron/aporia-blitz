#pragma once

#include "aporia_types.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #define OPENGL_VERSION_MAJOR 3
    #define OPENGL_VERSION_MINOR 0
    #define OPENGL_MAX_TEXTURE_UNITS 16
    #define OPENGL_SHADER_VERSION "#version 300 es"

    namespace Aporia
    {
        using texture_id = f32;
    }
#else
    #define OPENGL_VERSION_MAJOR 4
    #define OPENGL_VERSION_MINOR 5
    #define OPENGL_MAX_TEXTURE_UNITS 32
    #define OPENGL_SHADER_VERSION "#version 450"

    namespace Aporia
    {
        using texture_id = u32;
    }
#endif
