set(GL3W_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/gl3w/include")

add_library(gl3w STATIC ${PROJECT_SOURCE_DIR}/thirdparty/gl3w/src/gl3w.c)

target_include_directories(gl3w PUBLIC ${GL3W_INCLUDE_DIR})
