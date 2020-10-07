add_executable(example example.cpp
    menu_scene.hpp
    credits_scene.hpp
    game_scene.hpp
    player.hpp
    cannon.hpp
    end_scene.hpp)

target_link_libraries(example core)

target_include_directories(example
    PUBLIC PerlinNoise-master)

target_compile_definitions(example PRIVATE APORIA_IMPORT)

set_target_properties(example PROPERTIES
    USE_FOLDERS                         ON
    RUNTIME_OUTPUT_DIRECTORY            "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example"
    VS_DEBUGGER_WORKING_DIRECTORY       "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example")
