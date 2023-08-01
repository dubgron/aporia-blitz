list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(APORIA_DEBUGTOOLS        "Compile with debug tools"      ON)
option(APORIA_USE_UNITY_BUILD   "Compile using unity build"     ON)
option(APORIA_EMSCRIPTEN        "Compile with Emscripten"       OFF)

find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(stb REQUIRED)

if (NOT APORIA_EMSCRIPTEN)
    find_package(glfw3 REQUIRED)
    find_package(gl3w REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

add_library(core STATIC
    "aporia.hpp"
    "aporia_animation.cpp"
    "aporia_animation.hpp"
    "aporia_camera.cpp"
    "aporia_camera.hpp"
    "aporia_config.cpp"
    "aporia_config.hpp"
    "aporia_debug.cpp"
    "aporia_debug.hpp"
    "aporia_entity.cpp"
    "aporia_entity.hpp"
    "aporia_entry.hpp"
    "aporia_fonts.cpp"
    "aporia_fonts.hpp"
    "aporia_game.cpp"
    "aporia_game.hpp"
    "aporia_inputs.cpp"
    "aporia_inputs.hpp"
    "aporia_memory.cpp"
    "aporia_memory.hpp"
    "aporia_rendering.cpp"
    "aporia_rendering.hpp"
    "aporia_shaders.cpp"
    "aporia_shaders.hpp"
    "aporia_strings.cpp"
    "aporia_strings.hpp"
    "aporia_textures.cpp"
    "aporia_textures.hpp"
    "aporia_types.hpp"
    "aporia_utils.cpp"
    "aporia_utils.hpp"
    "aporia_window.cpp"
    "aporia_window.hpp"
    "aporia_world.cpp"
    "aporia_world.hpp"
    "editor/aporia_commandline.cpp"
    "platform/aporia_opengl.cpp"
    "platform/aporia_opengl.hpp"
    "platform/aporia_os.cpp"
    "platform/aporia_os.hpp"
    "platform/aporia_unix.cpp"
    "platform/aporia_win32.cpp")

set_source_files_properties(
    "platform/aporia_unix.cpp"
    "platform/aporia_win32.cpp"
    PROPERTIES HEADER_FILE_ONLY TRUE)

target_precompile_headers(core PRIVATE "aporia_pch.hpp")

target_link_libraries(core
    glfw
    glm
    imgui
    nlohmann_json
    spdlog
    stb)

if (NOT APORIA_EMSCRIPTEN)
    target_link_libraries(core
        gl3w
        OpenGL::GL)
endif()

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/core")

target_compile_definitions(core PUBLIC
    _USE_MATH_DEFINES
    NOMINMAX)

if (APORIA_DEBUGTOOLS)
    target_compile_definitions(core PUBLIC APORIA_DEBUGTOOLS)
endif()

# Add platform-specific defines
if (WIN32)
    target_compile_definitions(core PUBLIC APORIA_WINDOWS)
elseif (UNIX)
    target_compile_definitions(core PUBLIC APORIA_UNIX)
    if (APPLE)
        target_compile_definitions(core PUBLIC APORIA_APPLE)
    else()
        target_compile_definitions(core PUBLIC APORIA_LINUX)
    endif()
endif()

# Disable exceptions and RTTI
if (MSVC)
    string(REGEX REPLACE "/EHsc" "/EHsc-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_definitions(core PUBLIC _HAS_EXCEPTIONS=0)
else()
    target_compile_options(core PUBLIC -fno-exceptions -fno-rtti)
endif()

if (APORIA_EMSCRIPTEN)
    target_compile_options(core
        PUBLIC
            -fbracket-depth=1000
            -O2)

    target_link_options(core
        PUBLIC
            -sFULL_ES3
            -sUSE_GLFW=3
            -sWASM=1
            -sUSE_WEBGL2=1
            -sMIN_WEBGL_VERSION=2
            -sMAX_WEBGL_VERSION=2
            -sALLOW_MEMORY_GROWTH=1
            -sASYNCIFY=1
            -sASSERTIONS=1
            --no-heap-copy)

    target_compile_definitions(core PUBLIC APORIA_EMSCRIPTEN)
endif()

source_group(core               "core/.+\.[cht]pp")
source_group(core/editor        "core/editor/.+\.[cht]pp")
source_group(core/platform      "core/platform/.+\.[cht]pp")

set_target_properties(core PROPERTIES
    USE_FOLDERS     ON
    OUTPUT_NAME     "aporia-blitz")

if (APORIA_USE_UNITY_BUILD)
    # UNITY_BUILD_BATCH_SIZE of 0 means combining all sources for the target into a single unity file
    set_target_properties(core PROPERTIES
        UNITY_BUILD             ON
        UNITY_BUILD_MODE        BATCH
        UNITY_BUILD_BATCH_SIZE  0)
endif()

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(core PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        VS_DEBUGGER_WORKING_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core")
endif()
