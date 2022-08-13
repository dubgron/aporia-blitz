set(GL3W_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/gl3w/include")

message("Executing gl3w_gen.py")
execute_process(COMMAND python gl3w_gen.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/gl3w)

add_library(gl3w STATIC ${PROJECT_SOURCE_DIR}/thirdparty/gl3w/src/gl3w.c)

target_include_directories(gl3w PUBLIC ${GL3W_INCLUDE_DIR})
