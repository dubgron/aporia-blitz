#pragma once

/* Standard Library */
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(__cpp_lib_format)
    #include <format>
#else
    #include <ctime>
#endif

/* Third-Party Libraries */
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <magic_enum.hpp>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "platform/opengl.hpp"

/* Aporia Blitz */
#include "common.hpp"
