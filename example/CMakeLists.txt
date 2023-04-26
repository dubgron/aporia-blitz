add_executable(example example.cpp)

target_link_libraries(example core)

set_target_properties(example PROPERTIES
    USE_FOLDERS     ON)

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY            "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example"
        VS_DEBUGGER_WORKING_DIRECTORY       "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example")

    get_target_property(TARGET_SOURCE_DIR example SOURCE_DIR)

    add_custom_command(TARGET example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${TARGET_SOURCE_DIR}/content"
        "$<TARGET_FILE_DIR:example>/content")
else()
    set_target_properties(example PROPERTIES
        OUTPUT_NAME     "index"
        SUFFIX          ".html")
endif()
