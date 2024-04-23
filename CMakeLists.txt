cmake_minimum_required(VERSION 3.26)

set(CMAKE_CONFIGURATION_TYPES Debug Release Development)

set(CMAKE_C_FLAGS_DEVELOPMENT "${CMAKE_C_FLAGS_RELWITHDEBINFO}"
    CACHE STRING "Flags used by the C compiler during DEVELOPMENT builds." FORCE)
set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
    CACHE STRING "Flags used by the CXX compiler during DEVELOPMENT builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}"
    CACHE STRING "Flags used by the linker during DEVELOPMENT builds." FORCE)

project(aporia-blitz)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(APORIA_DEBUGTOOLS        "Compile with debug tools"      ON)
option(APORIA_EDITOR            "Compile with editor"           ON)
option(APORIA_USE_UNITY_BUILD   "Compile using unity build"     ON)
option(APORIA_EMSCRIPTEN        "Compile with Emscripten"       OFF)

if (APORIA_EDITOR AND APORIA_EMSCRIPTEN)
    message(FATAL_ERROR "Building editor with Emscripten is not supported!.")
endif()

if (APORIA_EDITOR OR APORIA_DEBUGTOOLS)
    set(APORIA_IMGUI TRUE)
endif()

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

if (APORIA_IMGUI)
    find_package(imgui REQUIRED)
endif()

if (NOT APORIA_EMSCRIPTEN)
    find_package(glfw3 REQUIRED)
    find_package(gl3w REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

set(APORIA_CORE_SOURCE_FILES
    "core/aporia_animations.cpp"
    "core/aporia_animations.hpp"
    "core/aporia_assets.cpp"
    "core/aporia_assets.hpp"
    "core/aporia_audio.cpp"
    "core/aporia_audio.hpp"
    "core/aporia_camera.cpp"
    "core/aporia_camera.hpp"
    "core/aporia_collision.cpp"
    "core/aporia_collision.hpp"
    "core/aporia_config.cpp"
    "core/aporia_config.hpp"
    "core/aporia_debug.cpp"
    "core/aporia_debug.hpp"
    "core/aporia_entity.cpp"
    "core/aporia_entity.hpp"
    "core/aporia_fonts.cpp"
    "core/aporia_fonts.hpp"
    "core/aporia_game.cpp"
    "core/aporia_game.hpp"
    "core/aporia_hash_table.hpp"
    "core/aporia_input.cpp"
    "core/aporia_input.hpp"
    "core/aporia_memory.cpp"
    "core/aporia_memory.hpp"
    "core/aporia_particles.cpp"
    "core/aporia_particles.hpp"
    "core/aporia_pch.hpp"
    "core/aporia_rendering.cpp"
    "core/aporia_rendering.hpp"
    "core/aporia_serialization.cpp"
    "core/aporia_serialization.hpp"
    "core/aporia_shaders.cpp"
    "core/aporia_shaders.hpp"
    "core/aporia_string.cpp"
    "core/aporia_string.hpp"
    "core/aporia_textures.cpp"
    "core/aporia_textures.hpp"
    "core/aporia_types.hpp"
    "core/aporia_utils.cpp"
    "core/aporia_utils.hpp"
    "core/aporia_window.cpp"
    "core/aporia_window.hpp"
    "core/aporia_world.cpp"
    "core/aporia_world.hpp"
    "core/platform/aporia_opengl.cpp"
    "core/platform/aporia_opengl.hpp"
    "core/platform/aporia_os.cpp"
    "core/platform/aporia_os.hpp"
    "core/platform/aporia_unix.cpp"
    "core/platform/aporia_win32.cpp")

if (APORIA_EDITOR)
    set(APORIA_EDITOR_SOURCE_FILES
        "core/editor/aporia_commandline.cpp"
        "core/editor/aporia_editor.cpp"
        "core/editor/aporia_editor.hpp")
endif()

add_executable(aporia "${APORIA_CORE_SOURCE_FILES}" "${APORIA_EDITOR_SOURCE_FILES}")

set_source_files_properties(
    "core/aporia_pch.hpp"
    "core/platform/aporia_unix.cpp"
    "core/platform/aporia_win32.cpp"
    PROPERTIES HEADER_FILE_ONLY TRUE)

target_precompile_headers(aporia PRIVATE "core/aporia_pch.hpp")

target_link_libraries(aporia
    glfw
    glm
    sokol
    stb)

if (APORIA_IMGUI)
    target_link_libraries(aporia imgui)
endif()

if (NOT APORIA_EMSCRIPTEN)
    target_link_libraries(aporia
        gl3w
        OpenGL::GL)
endif()

# Link platform-specific libraries
if (APORIA_LINUX AND NOT APORIA_EMSCRIPTEN)
    find_package(ALSA REQUIRED)
    target_link_libraries(aporia ALSA::ALSA)
elseif (APORIA_APPLE)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    target_link_libraries(aporia "${AUDIO_TOOLBOX_LIBRARY}")
endif()

target_compile_features(aporia PUBLIC cxx_std_20)
target_include_directories(aporia PUBLIC "${PROJECT_SOURCE_DIR}/core")
target_compile_definitions(aporia PUBLIC _USE_MATH_DEFINES)

if (APORIA_DEBUGTOOLS)
    target_compile_definitions(aporia PUBLIC APORIA_DEBUGTOOLS)
endif()

if (APORIA_EDITOR)
    target_compile_definitions(aporia PUBLIC APORIA_EDITOR)
endif()

if (APORIA_IMGUI)
    target_compile_definitions(aporia PUBLIC APORIA_IMGUI)
endif()

# Add platform-specific defines
if (APORIA_WINDOWS)
    target_compile_definitions(aporia PUBLIC APORIA_WINDOWS)
elseif (APORIA_UNIX)
    target_compile_definitions(aporia PUBLIC APORIA_UNIX)
    if (APORIA_APPLE)
        target_compile_definitions(aporia PUBLIC APORIA_APPLE)
    elseif (APORIA_LINUX)
        target_compile_definitions(aporia PUBLIC APORIA_LINUX)
    endif()
endif()

# Disable exceptions and RTTI
if (MSVC)
    string(REGEX REPLACE "/EHsc" "/EHsc-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_definitions(aporia PUBLIC _HAS_EXCEPTIONS=0)
else()
    target_compile_options(aporia PUBLIC -fno-exceptions -fno-rtti)
endif()

if (APORIA_EMSCRIPTEN)
    target_compile_options(aporia
        PUBLIC
            -fbracket-depth=1000
            -O2)

    target_link_options(aporia
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

    target_compile_definitions(aporia PUBLIC APORIA_EMSCRIPTEN)
endif()

source_group(core               "core/.+\.[cht]pp")
source_group(core/editor        "core/editor/.+\.[cht]pp")
source_group(core/platform      "core/platform/.+\.[cht]pp")

set_target_properties(aporia PROPERTIES
    USE_FOLDERS                 ON
    OUTPUT_NAME_RELEASE         "aporia"
    OUTPUT_NAME_DEBUG           "aporia_debug"
    OUTPUT_NAME_DEVELOPMENT     "aporia_dev")

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(aporia PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE            "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG              "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEVELOPMENT        "${PROJECT_SOURCE_DIR}/bin"
        VS_DEBUGGER_WORKING_DIRECTORY               "${PROJECT_SOURCE_DIR}/bin")
else()
    set_target_properties(aporia PROPERTIES
        OUTPUT_NAME                 "index"
        SUFFIX                      ".html"
        RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/emscripten/bin")
endif()

if (APORIA_USE_UNITY_BUILD)
    # UNITY_BUILD_BATCH_SIZE of 0 means combining all sources for the target into a single unity file
    set_target_properties(aporia PROPERTIES
        UNITY_BUILD             ON
        UNITY_BUILD_MODE        BATCH
        UNITY_BUILD_BATCH_SIZE  0)
endif()

if (MSVC)
    target_sources(aporia PRIVATE "${PROJECT_SOURCE_DIR}/misc/aporia_core.natvis")

    # Remove old .pdb files
    add_custom_command(TARGET aporia PRE_BUILD
        COMMAND "${CMAKE_COMMAND}"
        ARGS -E remove -f "$<TARGET_PDB_FILE:aporia>")

    # Disable incremental linking
    target_link_options(aporia PUBLIC "/INCREMENTAL:NO")
endif()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT aporia)

file(GLOB APORIA_GIT_HOOKS "${PROJECT_SOURCE_DIR}/.githooks/*")
file(COPY ${APORIA_GIT_HOOKS} DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks/")
