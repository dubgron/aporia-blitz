cmake_minimum_required(VERSION 3.15)

project(aporia-blitz)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (WIN32)
    add_compile_definitions(APORIA_WINDOWS)
elseif (UNIX)
    add_compile_definitions(APORIA_UNIX)
    if (APPLE)
        add_compile_definitions(APORIA_APPLE)
    else ()
        add_compile_definitions(APORIA_LINUX)
    endif ()
endif ()

add_subdirectory(core)
add_subdirectory(example)

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT example)