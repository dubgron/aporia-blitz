if(NOT DEFINED ENV{NLOHMANN_JSON_BUILD})
    message(FATAL_ERROR "Environmental variable NLOHMANN_JSON_BUILD is not set!")
endif()

find_package(nlohmann_json 3.7.0 REQUIRED PATHS $ENV{NLOHMANN_JSON_BUILD})

if(NOT DEFINED ENV{SFML_BUILD})
    message(FATAL_ERROR "Environmental variable SFML_BUILD is not set!")
endif()

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR $ENV{SFML_BUILD})
set(SFML_ROOT "${PROJECT_SOURCE_DIR}/thirdparty/SFML/extlibs/libs-msvc-universal/x64")
find_package(SFML 2.5.1 REQUIRED COMPONENTS 
    graphics
    audio
    window
    system)
    
if(NOT DEFINED ENV{SPDLOG_BUILD})
    message(FATAL_ERROR "Environmental variable SPDLOG_BUILD is not set!")
endif()

find_package(spdlog REQUIRED PATHS $ENV{SPDLOG_BUILD})

if(NOT DEFINED ENV{MAGIC_ENUM_BUILD})
    message(FATAL_ERROR "Environmental variable MAGIC_ENUM_BUILD is not set!")
endif()

find_package(magic_enum REQUIRED PATHS $ENV{MAGIC_ENUM_BUILD})

add_library(core SHARED
    "aporia.hpp"
    "configs.hpp"
    "config_manager.cpp"
    "config_manager.hpp"
    "engine.cpp"
    "engine.hpp"
    "entry_point.hpp"
    "event.hpp"
    "event_manager.cpp"
    "event_manager.hpp"
    "event_manager.tpp"
    "input_buffer.hpp"
    "input_manager.cpp"
    "input_manager.hpp"
    "inputs.hpp"
    "log_buffer.hpp"
    "logger.cpp"
    "logger.hpp"
    "platform.hpp"
    "renderer.cpp"
    "renderer.hpp"
    "sprite.hpp"
    "texture.hpp"
    "texture_manager.cpp"
    "texture_manager.hpp"    
    "vertex_array.cpp"
    "vertex_array.hpp"
    "window.cpp"
    "window.hpp"
    "utils/read_file.hpp"
    "configs/texture_config.hpp"
    "configs/window_config.hpp"
    "inputs/keyboard.hpp"
    "inputs/mouse.hpp"
    "utils/type_traits.hpp")

target_link_libraries(core 
    sfml-graphics
    sfml-audio
    sfml-window
    sfml-system
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum)

target_include_directories(core 
    PUBLIC
        "${PROJECT_SOURCE_DIR}/core"
    PRIVATE
        "${PROJECT_SOURCE_DIR}/thirdparty/nlohmann_json/single_include")
    
target_compile_features(core PUBLIC cxx_std_17)
    
target_compile_definitions(core 
    PUBLIC
        SFML_STATIC
    PRIVATE
        APORIA_EXPORT 
        __PROJECT__="core")

source_group(core FILES
    "aporia.hpp"
    "configs.hpp"
    "config_manager.cpp"
    "config_manager.hpp"
    "engine.cpp"
    "engine.hpp"
    "entry_point.hpp"
    "event.hpp"
    "event_manager.cpp"
    "event_manager.hpp"
    "event_manager.tpp"
    "input_buffer.hpp"
    "input_manager.cpp"
    "input_manager.hpp"
    "inputs.hpp"
    "log_buffer.hpp"
    "logger.cpp"
    "logger.hpp"
    "platform.hpp"
    "renderer.cpp"
    "renderer.hpp"
    "sprite.hpp"
    "texture.hpp"
    "texture_manager.cpp"
    "texture_manager.hpp"
    "vertex_array.cpp"
    "vertex_array.hpp"
    "window.cpp"
    "window.hpp")

source_group(core\\config FILES
    "configs/texture_config.hpp"
    "configs/window_config.hpp")

source_group(core\\inputs FILES
    "inputs/keyboard.hpp"
    "inputs/mouse.hpp")

source_group(core\\utils FILES
    "utils/read_file.hpp"
    "utils/type_traits.hpp")

set_target_properties(core PROPERTIES
    USE_FOLDERS                       ON
    OUTPUT_NAME                       "aporia-blitz"
    RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core"
    VS_DEBUGGER_WORKING_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/core")
