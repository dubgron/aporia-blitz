add_executable(example example.cpp)

target_link_libraries(example core)

set_target_properties(example PROPERTIES
    USE_FOLDERS                 ON
    OUTPUT_NAME_RELEASE         "example"
    OUTPUT_NAME_DEBUG           "example_debug"
    OUTPUT_NAME_RELWITHDEBINFO  "example_dev"
    OUTPUT_NAME_MINSIZEREL      "example_minsize")

if (NOT APORIA_EMSCRIPTEN)
    set_target_properties(example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE            "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG              "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO     "${PROJECT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL         "${PROJECT_SOURCE_DIR}/bin"
        VS_DEBUGGER_WORKING_DIRECTORY               "${PROJECT_SOURCE_DIR}/bin")
else()
    set_target_properties(example PROPERTIES
        OUTPUT_NAME                 "index"
        SUFFIX                      ".html"
        RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/emscripten/bin")
endif()
