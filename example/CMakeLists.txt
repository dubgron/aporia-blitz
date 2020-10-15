add_executable(example example.cpp)

target_link_libraries(example core)

target_compile_definitions(example
    PRIVATE
        APORIA_IMPORT)

set_target_properties(example PROPERTIES
    USE_FOLDERS     ON)

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY            "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example"
        VS_DEBUGGER_WORKING_DIRECTORY       "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/example")
else()
    set_target_properties(example PROPERTIES
        OUTPUT_NAME     "index"
        SUFFIX          ".html")
endif()
