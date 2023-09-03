#pragma once

// Standard Library
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <string_view>
#include <unordered_map>


// Third-Party Libraries
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/matrix_transform.hpp>

#if defined(APORIA_DEBUGTOOLS)
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
#include "platform/aporia_opengl.hpp"

#if defined(APORIA_WINDOWS)
    #include <windows.h>
#elif defined(APORIA_UNIX)
    #include <dlfcn.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/types.h>
#else
    #error OS not supported!
#endif
