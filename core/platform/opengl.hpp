#pragma once

#include "platform/opengl_common.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <GLES3/gl3.h>
#else
    #include <GL/gl3w.h>
#endif

#include <GLFW/glfw3.h>
