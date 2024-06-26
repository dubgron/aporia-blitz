#pragma once

// C Standard Library
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C++ Standard Library
#include <chrono>
#include <random>

// Third-Party Libraries
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/matrix_transform.hpp>

#if defined(APORIA_IMGUI)
    #include <imgui.h>
    #include <backends/imgui_impl_glfw.h>
    #include <backends/imgui_impl_opengl3.h>
#else
    #define IMGUI_DISABLE 1
#endif

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

// Platform
#if defined(APORIA_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#elif defined(APORIA_UNIX)
    #include <dlfcn.h>
    #include <pthread.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/types.h>
#else
    #error OS not supported!
#endif

#include "platform/aporia_opengl.hpp"
