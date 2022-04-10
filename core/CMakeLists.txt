list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(APORIA_EMSCRIPTEN "Compile with Emscripten" OFF)

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
    "animation_manager.cpp"
    "animation_manager.hpp"
    "aporia.hpp"
    "camera.cpp"
    "camera.hpp"
    "camera_controller.cpp"
    "camera_controller.hpp"
    "config_manager.cpp"
    "config_manager.hpp"
    "entity.hpp"
    "entry_point.hpp"
    "event.hpp"
    "event_manager.hpp"
    "event_manager.tpp"
    "font_manager.cpp"
    "font_manager.hpp"
    "game.cpp"
    "game.hpp"
    "input_buffer.hpp"
    "input_manager.cpp"
    "input_manager.hpp"
    "log_buffer.hpp"
    "logger.cpp"
    "logger.hpp"
    "scene.hpp"
    "scene_manager.cpp"
    "scene_manager.hpp"
    "shader_manager.cpp"
    "shader_manager.hpp"
    "texture_manager.cpp"
    "texture_manager.hpp"
    "window.cpp"
    "window.hpp"
    "components/circular.hpp"
    "components/color.hpp"
    "components/linear2d.hpp"
    "components/rectangular.hpp"
    "components/texture.hpp"
    "components/transform2d.hpp"
    "configs/all_configs.hpp"
    "configs/animation_config.hpp"
    "configs/camera_config.hpp"
    "configs/texture_config.hpp"
    "configs/window_config.hpp"
    "graphics/circle2d.hpp"
    "graphics/common.hpp"
    "graphics/font.hpp"
    "graphics/group.hpp"
    "graphics/image.cpp"
    "graphics/image.hpp"
    "graphics/index_buffer.hpp"
    "graphics/line2d.hpp"
    "graphics/opengl.hpp"
    "graphics/rectangle2d.hpp"
    "graphics/renderer.cpp"
    "graphics/renderer.hpp"
    "graphics/render_queue.hpp"
    "graphics/shader.hpp"
    "graphics/sprite.hpp"
    "graphics/text.cpp"
    "graphics/text.hpp"
    "graphics/vertex.hpp"
    "graphics/vertex_array.hpp"
    "graphics/vertex_buffer.hpp"
    "graphics/animations/animation.cpp"
    "graphics/animations/animation.hpp"
    "graphics/animations/animator.cpp"
    "graphics/animations/animator.hpp"
    "inputs/all_inputs.hpp"
    "inputs/keyboard.hpp"
    "inputs/mouse.hpp"
    "layers/imgui_layer.cpp"
    "layers/imgui_layer.hpp"
    "layers/layer.hpp"
    "layers/layer_stack.cpp"
    "layers/layer_stack.hpp"
    "utils/color_conversion.hpp"
    "utils/math.hpp"
    "utils/random.hpp"
    "utils/read_file.hpp"
    "utils/timer.hpp"
    "utils/type_traits.hpp")

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

    target_compile_features(core PUBLIC cxx_std_20)
endif()

target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/core")

target_compile_definitions(core
    PUBLIC
        _USE_MATH_DEFINES
        MAGIC_ENUM_RANGE_MAX=512
        STB_IMAGE_IMPLEMENTATION
    PRIVATE
        APORIA_EXPORT)

if (APORIA_EMSCRIPTEN)
    target_compile_definitions(core PUBLIC APORIA_EMSCRIPTEN)
endif()

source_group(core                           "core/.+\.[cht]pp")
source_group(core\\components               "core/components/.+\.[cht]pp")
source_group(core\\configs                  "core/configs/.+\.[cht]pp")
source_group(core\\graphics                 "core/graphics/.+\.[cht]pp")
source_group(core\\graphics\\animations     "core/graphics/animations.+\.[cht]pp")
source_group(core\\inputs                   "core/inputs/.+\.[cht]pp")
source_group(core\\layers                   "core/layers/.+\.[cht]pp")
source_group(core\\utils                    "core/utils/.+\.[cht]pp")

set_target_properties(core PROPERTIES
    USE_FOLDERS     ON
    OUTPUT_NAME     "aporia-blitz")

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(core PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
        VS_DEBUGGER_WORKING_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core")
endif()
