function(validate_env name)
    if(NOT DEFINED ENV{${name}})
        message(FATAL_ERROR "Environmental variable ${name} is not set!")
    endif()
endfunction()

validate_env("NLOHMANN_JSON_BUILD")
find_package(nlohmann_json 3.7.0 REQUIRED PATHS $ENV{NLOHMANN_JSON_BUILD})

validate_env("SFML_BUILD")
set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.5.1 REQUIRED PATHS $ENV{SFML_BUILD} COMPONENTS graphics audio window system)

validate_env("SPDLOG_BUILD")
find_package(spdlog REQUIRED PATHS $ENV{SPDLOG_BUILD} NO_DEFAULT_PATH)

validate_env("MAGIC_ENUM_BUILD")
find_package(magic_enum REQUIRED PATHS $ENV{MAGIC_ENUM_BUILD})

validate_env("GL3W_BUILD")
find_package(gl3w REQUIRED PATHS $ENV{GL3W_BUILD})

validate_env("GLFW_BUILD")
find_package(glfw3 REQUIRED PATHS $ENV{GLFW_BUILD})

set(glm_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glm/cmake/glm/")
find_package(glm REQUIRED)

find_package(OpenGL REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(ImGui REQUIRED)

find_package(stb REQUIRED)

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
    "graphics/group.hpp"
    "graphics/line2d.hpp"
    "graphics/rectangle2d.hpp"
    "graphics/renderer.cpp"
    "graphics/renderer.hpp"
    "graphics/renderer.tpp"
    "graphics/sprite.hpp"
    "graphics/vertex_array.hpp"
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
    OpenGL::GL
    sfml-graphics
    sfml-audio
    sfml-window
    sfml-system
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    imgui
    glm::glm
    glfw
    gl3w
    stb)

target_include_directories(core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/core")

target_compile_features(core PUBLIC cxx_std_20)

target_compile_definitions(core
    PUBLIC
        SFML_STATIC
        _USE_MATH_DEFINES
        MAGIC_ENUM_RANGE_MAX=512
    PRIVATE
        APORIA_EXPORT)

source_group(core REGULAR_EXPRESSION "core/.+\.[cht]pp")
source_group(core\\components REGULAR_EXPRESSION "core/components/.+\.[cht]pp")
source_group(core\\configs REGULAR_EXPRESSION "core/configs/.+\.[cht]pp")
source_group(core\\graphics REGULAR_EXPRESSION "core/graphics/.+\.[cht]pp")
source_group(core\\graphics\\animations REGULAR_EXPRESSION "core/graphics/animations.+\.[cht]pp")
source_group(core\\inputs REGULAR_EXPRESSION "core/inputs/.+\.[cht]pp")
source_group(core\\layers REGULAR_EXPRESSION "core/layers/.+\.[cht]pp")
source_group(core\\utils REGULAR_EXPRESSION "core/utils/.+\.[cht]pp")

set_target_properties(core PROPERTIES
    USE_FOLDERS                       ON
    OUTPUT_NAME                       "aporia-blitz"
    ARCHIVE_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
    RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
    VS_DEBUGGER_WORKING_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core")
