cmake_minimum_required(VERSION 3.26)

project(aporia-blitz)

add_subdirectory(core)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT core)

file(GLOB APORIA_GIT_HOOKS "${PROJECT_SOURCE_DIR}/.githooks/*")
file(COPY ${APORIA_GIT_HOOKS} DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks/")
