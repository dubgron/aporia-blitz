#pragma once

#include "debug/logger.hpp"
#include "platform/opengl_common.hpp"

#if defined(APORIA_EMSCRIPTEN)

    #include <GLES3/gl3.h>

#else

    #include <GL/gl3w.h>

    namespace Aporia
    {
        constexpr LogLevel gl_debug_severity_to_log_level(GLenum severity)
        {
            switch (severity)
            {
                case GL_DEBUG_SEVERITY_HIGH:            return LogLevel::Error;
                case GL_DEBUG_SEVERITY_MEDIUM:          return LogLevel::Warning;
                case GL_DEBUG_SEVERITY_LOW:             return LogLevel::Info;
                case GL_DEBUG_SEVERITY_NOTIFICATION:    return LogLevel::Verbose;
                default:                                return LogLevel::Critical;
            }
        }

        constexpr const char* gl_debug_source_to_string(GLenum source)
        {
            switch (source)
            {
                case GL_DEBUG_SOURCE_API:                   return "OPENGL_API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:         return "WINDOW_SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:       return "SHADER_COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY:           return "THIRD_PARTY";
                case GL_DEBUG_SOURCE_APPLICATION:           return "APPLICATION";
                case GL_DEBUG_SOURCE_OTHER:                 return "OTHER";
                default:                                    return "INVALID_SOURCE";
            }
        }

        constexpr const char* gl_debug_type_to_string(GLenum type)
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
                default:                                    return "INVALID_TYPE";
            }
        }

        constexpr const char* gl3w_return_code_to_string(i32 gl3w_return_code)
        {
            switch (gl3w_return_code)
            {
                case GL3W_OK:                       return "OK";
                case GL3W_ERROR_INIT:               return "ERROR INIT";
                case GL3W_ERROR_LIBRARY_OPEN:       return "ERROR LIBRARY OPEN";
                case GL3W_ERROR_OPENGL_VERSION:     return "ERROR OPENGL VERSION";
                default:                            return "INVALID RETURN CODE";
            }
        }
    }

#endif

#include <GLFW/glfw3.h>
