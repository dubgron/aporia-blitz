set(WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(example example.cpp level_manager.hpp)

target_compile_definitions(example 
    PRIVATE 
        APORIA_IMPORT)

target_link_libraries(example core)

set_target_properties(example PROPERTIES
    USE_FOLDERS                         ON
    RUNTIME_OUTPUT_DIRECTORY            "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example"
    VS_DEBUGGER_WORKING_DIRECTORY       "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example")

add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:core>
    $<TARGET_FILE_DIR:example>)
    