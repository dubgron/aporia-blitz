list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(APORIA_DEBUGTOOLS    "Compile with debug tools"  ON)
option(APORIA_EMSCRIPTEN    "Compile with Emscripten"   OFF)

find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(magic_enum REQUIRED)
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
    "aporia_config.cpp"
    "aporia_config.hpp"
    "aporia_inputs.cpp"
    "aporia_inputs.hpp"
    "aporia_memory.cpp"
    "aporia_memory.hpp"
    "aporia_shaders.cpp"
    "aporia_shaders.hpp"
    "aporia_textures.cpp"
    "aporia_textures.hpp"
    "aporia_types.hpp"
    "common.hpp"
    "entry_point.hpp"
    "font_manager.cpp"
    "font_manager.hpp"
    "game.cpp"
    "game.hpp"
    "scene.hpp"
    "scene_manager.cpp"
    "scene_manager.hpp"
    "window.cpp"
    "window.hpp"
    "world.cpp"
    "world.hpp"
    "components/color.hpp"
    "components/rectangular.hpp"
    "components/transform2d.hpp"
    "debug/asserts.hpp"
    "debug/break.hpp"
    "debug/logger.cpp"
    "debug/logger.hpp"
    "graphics/camera.cpp"
    "graphics/camera.hpp"
    "graphics/camera_controller.cpp"
    "graphics/camera_controller.hpp"
    "graphics/font.hpp"
    "graphics/framebuffer.cpp"
    "graphics/framebuffer.hpp"
    "graphics/index_buffer.cpp"
    "graphics/index_buffer.hpp"
    "graphics/light.hpp"
    "graphics/renderer.cpp"
    "graphics/renderer.hpp"
    "graphics/render_queue.hpp"
    "graphics/uniform_buffer.hpp"
    "graphics/vertex.hpp"
    "graphics/vertex_array.cpp"
    "graphics/vertex_array.hpp"
    "graphics/vertex_buffer.cpp"
    "graphics/vertex_buffer.hpp"
    "graphics/drawables/circle2d.hpp"
    "graphics/drawables/drawables.cpp"
    "graphics/drawables/group.hpp"
    "graphics/drawables/line2d.hpp"
    "graphics/drawables/rectangle2d.hpp"
    "graphics/drawables/sprite.hpp"
    "graphics/drawables/text.hpp"
    "layers/imgui_layer.cpp"
    "layers/imgui_layer.hpp"
    "layers/layer.hpp"
    "layers/layer_stack.cpp"
    "layers/layer_stack.hpp"
    "platform/opengl.hpp"
    "platform/opengl_common.hpp"
    "utils/color_conversion.hpp"
    "utils/math.hpp"
    "utils/random.hpp"
    "utils/read_file.hpp"
    "utils/timer.hpp"
    "utils/type_traits.hpp")

target_precompile_headers(core PRIVATE "aporia_pch.hpp")

target_link_libraries(core
    glfw
    glm
    imgui
    magic_enum
    nlohmann_json
    spdlog
    stb)

if (NOT APORIA_EMSCRIPTEN)
    target_link_libraries(core
        gl3w
        OpenGL::GL)
endif()

target_compile_features(core PUBLIC cxx_std_23)

target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/core")

target_compile_definitions(core
    PUBLIC
        _USE_MATH_DEFINES
        MAGIC_ENUM_RANGE_MAX=512
        NOMINMAX)

if (APORIA_DEBUGTOOLS)
    target_compile_definitions(core PUBLIC APORIA_DEBUGTOOLS)
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

source_group(core                       "core/.+\.[cht]pp")
source_group(core/components            "core/components/.+\.[cht]pp")
source_group(core/debug                 "core/debug/.+\.[cht]pp")
source_group(core/graphics              "core/graphics/.+\.[cht]pp")
source_group(core/graphics/drawables    "core/graphics/drawables.+\.[cht]pp")
source_group(core/layers                "core/layers/.+\.[cht]pp")
source_group(core/platform              "core/platform/.+\.[cht]pp")
source_group(core/utils                 "core/utils/.+\.[cht]pp")

set_target_properties(core PROPERTIES
    USE_FOLDERS     ON
    OUTPUT_NAME     "aporia-blitz")

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(core PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        VS_DEBUGGER_WORKING_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core")
endif()
