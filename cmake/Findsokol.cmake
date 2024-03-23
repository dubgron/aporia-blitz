set(SOKOL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/sokol")

add_library(sokol INTERFACE)

target_include_directories(sokol INTERFACE ${SOKOL_INCLUDE_DIR})
