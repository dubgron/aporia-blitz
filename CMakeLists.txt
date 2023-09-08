cmake_minimum_required(VERSION 3.26)

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

file(GLOB APORIA_GIT_HOOKS "${PROJECT_SOURCE_DIR}/.githooks/*")
file(COPY ${APORIA_GIT_HOOKS} DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks/")
