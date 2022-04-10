#pragma once

#if defined(APORIA_EMSCRIPTEN)
#   define IMGUI_OPENGL_VERSION "#version 300 es"
#   define OPENGL_VERSION_MAJOR 3
#   define OPENGL_VERSION_MINOR 0
#   define OPENGL_MAX_TEXTURE_UNITS 16
#   include <GLES3/gl3.h>
#else
#   define IMGUI_OPENGL_VERSION "#version 450"
#   define OPENGL_VERSION_MAJOR 4
#   define OPENGL_VERSION_MINOR 5
#   define OPENGL_MAX_TEXTURE_UNITS 32
#   include <GL/gl3w.h>

    namespace Aporia
    {
        static constexpr const char* get_debug_source(GLenum source)
        {
            switch (source)
            {
            case GL_DEBUG_SOURCE_API:                   return "OPENGL_API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:         return "WINDOW_SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:       return "SHADER_COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:           return "THIRD_PARTY";
            case GL_DEBUG_SOURCE_APPLICATION:           return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER:                 return "OTHER";
            default:                                    return "NONE";
            }
        }

        static constexpr const char* get_debug_type(GLenum type)
        {
            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR:                   return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:     return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:      return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY:             return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE:             return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:                  return "MARKER";
            case GL_DEBUG_TYPE_PUSH_GROUP:              return "PUSH_GROUP";
            case GL_DEBUG_TYPE_POP_GROUP:               return "POP_GROUP";
            case GL_DEBUG_TYPE_OTHER:                   return "OTHER";
            default:                                    return "NONE";
            }
        }
    }
#endif
