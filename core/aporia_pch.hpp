#pragma once

// Standard Library
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <string_view>
#include <unordered_map>

#if defined(__cpp_lib_format)
    #include <format>
#else
    #include <ctime>
#endif

// Third-Party Libraries
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

// Platform
#include "platform/aporia_opengl.hpp"
