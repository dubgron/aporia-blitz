cmake_minimum_required(VERSION 3.15)

project(aporia-blitz)

option(APORIA_BUILD_CORE    "Build Aporia Core"    ON)
option(APORIA_BUILD_EXAMPLE "Build Aporia Example" ON)

if (APORIA_BUILD_CORE)
    add_subdirectory(core)
endif()

if (APORIA_BUILD_EXAMPLE)
    add_subdirectory(example)
    set_directory_properties(PROPERTIES VS_STARTUP_PROJECT example)
endif()
