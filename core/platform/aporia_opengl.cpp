#include "aporia_opengl.hpp"

#include "aporia_debug.hpp"

namespace Aporia
{
#if !defined(APORIA_EMSCRIPTEN)
    static String gl3w_return_code_to_string(i32 gl3w_return_code)
    {
        switch (gl3w_return_code)
        {
            case GL3W_OK:                               return "OK";
            case GL3W_ERROR_INIT:                       return "ERROR INIT";
            case GL3W_ERROR_LIBRARY_OPEN:               return "ERROR LIBRARY OPEN";
            case GL3W_ERROR_OPENGL_VERSION:             return "ERROR OPENGL VERSION";
            default:                                    return "INVALID RETURN CODE";
        }
    }
#endif

#if defined(APORIA_DEBUGTOOLS)
    static LogLevel gl_debug_severity_to_log_level(GLenum severity)
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:                return LogLevel::Error;
            case GL_DEBUG_SEVERITY_MEDIUM:              return LogLevel::Warning;
            case GL_DEBUG_SEVERITY_LOW:                 return LogLevel::Info;
            case GL_DEBUG_SEVERITY_NOTIFICATION:        return LogLevel::Garbage;
            default:                                    return LogLevel::Critical;
        }
    }

    static String gl_debug_source_to_string(GLenum source)
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

    static String gl_debug_type_to_string(GLenum type)
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

    static void gl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, CString message, const void* userParam)
    {
        LogLevel log_level = gl_debug_severity_to_log_level(severity);
        String debug_source = gl_debug_source_to_string(source);
        String debug_type = gl_debug_type_to_string(type);
        APORIA_LOG(log_level, "% % [ID: %] '%'", debug_source, debug_type, id, message);
    }
#endif

    void opengl_init()
    {
#if !defined(APORIA_EMSCRIPTEN)
        // @NOTE(dubgron): It has to be called after glfwMakeContextCurrent.
        i32 gl3w_init_return_code = gl3wInit();

        APORIA_ASSERT_WITH_MESSAGE(gl3w_init_return_code == GL3W_OK,
            "Failed to initialize OpenGL! Reason: %", gl3w_return_code_to_string(gl3w_init_return_code));
#endif

#if defined(APORIA_DEBUGTOOLS) && !defined(APORIA_EMSCRIPTEN)
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(gl_debug_message_callback, nullptr);
#endif

        APORIA_LOG(Info, (CString)glGetString(GL_VERSION));
    }
}
