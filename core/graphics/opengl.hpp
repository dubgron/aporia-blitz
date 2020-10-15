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
#endif
