#pragma once

#if defined(APORIA_EMSCRIPTEN)

    #include <GLES3/gl32.h>

    #define OPENGL_VERSION_MAJOR 3
    #define OPENGL_VERSION_MINOR 0
    #define OPENGL_MAX_TEXTURE_UNITS 16
    #define OPENGL_SHADER_VERSION "#version 300 es"

#else

    #include <GL/gl3w.h>

    #define OPENGL_VERSION_MAJOR 4
    #define OPENGL_VERSION_MINOR 5
    #define OPENGL_MAX_TEXTURE_UNITS 32
    #define OPENGL_SHADER_VERSION "#version 450"

    #include "aporia_debug.hpp"

    namespace Aporia
    {
        LogLevel gl_debug_severity_to_log_level(GLenum severity);
        const char* gl_debug_source_to_string(GLenum source);
        const char* gl_debug_type_to_string(GLenum type);
        const char* gl3w_return_code_to_string(i32 gl3w_return_code);
    }

#endif

#include <GLFW/glfw3.h>

namespace Aporia
{
    // @NOTE(dubgron): It has to be called after creating a window.
    void opengl_init();
}
