list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(APORIA_DEBUGTOOLS        "Compile with debug tools"      ON)
option(APORIA_USE_UNITY_BUILD   "Compile using unity build"     ON)
option(APORIA_EMSCRIPTEN        "Compile with Emscripten"       OFF)

if (WIN32)
    set(APORIA_WINDOWS TRUE)
elseif (UNIX)
    set(APORIA_UNIX TRUE)
    if (APPLE)
        set(APORIA_APPLE TRUE)
    else()
        set(APORIA_LINUX TRUE)
    endif()
endif()


find_package(glm REQUIRED)
find_package(sokol REQUIRED)
find_package(stb REQUIRED)

if (APORIA_DEBUGTOOLS)
    find_package(imgui REQUIRED)
endif()

if (NOT APORIA_EMSCRIPTEN)
    find_package(glfw3 REQUIRED)
    find_package(gl3w REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

add_executable(core
    "aporia_animations.cpp"
    "aporia_animations.hpp"
    "aporia_assets.cpp"
    "aporia_assets.hpp"
    "aporia_audio.cpp"
    "aporia_audio.hpp"
    "aporia_camera.cpp"
    "aporia_camera.hpp"
    "aporia_collision.cpp"
    "aporia_collision.hpp"
    "aporia_config.cpp"
    "aporia_config.hpp"
    "aporia_debug.cpp"
    "aporia_debug.hpp"
    "aporia_entity.cpp"
    "aporia_entity.hpp"
    "aporia_fonts.cpp"
    "aporia_fonts.hpp"
    "aporia_game.cpp"
    "aporia_game.hpp"
    "aporia_hash_table.hpp"
    "aporia_input.cpp"
    "aporia_input.hpp"
    "aporia_memory.cpp"
    "aporia_memory.hpp"
    "aporia_particles.cpp"
    "aporia_particles.hpp"
    "aporia_pch.hpp"
    "aporia_rendering.cpp"
    "aporia_rendering.hpp"
    "aporia_serialization.cpp"
    "aporia_serialization.hpp"
    "aporia_shaders.cpp"
    "aporia_shaders.hpp"
    "aporia_string.cpp"
    "aporia_string.hpp"
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
    "aporia_pch.hpp"
    "platform/aporia_unix.cpp"
    "platform/aporia_win32.cpp"
    PROPERTIES HEADER_FILE_ONLY TRUE)

target_precompile_headers(core PRIVATE "aporia_pch.hpp")

target_link_libraries(core
    glfw
    glm
    sokol
    stb)

if (APORIA_DEBUGTOOLS)
    target_link_libraries(core imgui)
endif()

if (NOT APORIA_EMSCRIPTEN)
    target_link_libraries(core
        gl3w
        OpenGL::GL)
endif()

# Link platform-specific libraries
if (APORIA_LINUX AND NOT APORIA_EMSCRIPTEN)
    find_package(ALSA REQUIRED)
    target_link_libraries(core ALSA::ALSA)
elseif (APORIA_APPLE)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    target_link_libraries(core "${AUDIO_TOOLBOX_LIBRARY}")
endif()

target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/core")
target_compile_definitions(core PUBLIC _USE_MATH_DEFINES)

if (APORIA_DEBUGTOOLS)
    target_compile_definitions(core PUBLIC APORIA_DEBUGTOOLS)
endif()

# Add platform-specific defines
if (APORIA_WINDOWS)
    target_compile_definitions(core PUBLIC APORIA_WINDOWS)
elseif (APORIA_UNIX)
    target_compile_definitions(core PUBLIC APORIA_UNIX)
    if (APORIA_APPLE)
        target_compile_definitions(core PUBLIC APORIA_APPLE)
    elseif (APORIA_LINUX)
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
    USE_FOLDERS                 ON
    OUTPUT_NAME_RELEASE         "aporia"
    OUTPUT_NAME_DEBUG           "aporia_debug"
    OUTPUT_NAME_RELWITHDEBINFO  "aporia_dev"
    OUTPUT_NAME_MINSIZEREL      "aporia_minsize")

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(core PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE            "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG              "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO     "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL         "${PROJECT_SOURCE_DIR}/bin"
        VS_DEBUGGER_WORKING_DIRECTORY               "${PROJECT_SOURCE_DIR}/bin")
else()
    set_target_properties(core PROPERTIES
        OUTPUT_NAME                 "index"
        SUFFIX                      ".html"
        RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/emscripten/bin")
endif()

if (APORIA_USE_UNITY_BUILD)
    # UNITY_BUILD_BATCH_SIZE of 0 means combining all sources for the target into a single unity file
    set_target_properties(core PROPERTIES
        UNITY_BUILD             ON
        UNITY_BUILD_MODE        BATCH
        UNITY_BUILD_BATCH_SIZE  0)
endif()

if (MSVC)
    target_sources(core PRIVATE "${PROJECT_SOURCE_DIR}/misc/aporia_core.natvis")
endif()
