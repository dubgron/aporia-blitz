set(IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/imgui")

add_library(imgui STATIC
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMGUI_INCLUDE_DIR}/examples/imgui_impl_glfw.cpp
    ${IMGUI_INCLUDE_DIR}/examples/imgui_impl_opengl3.cpp)

target_link_libraries(imgui
    OpenGL::GL
    glfw
    gl3w)

target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
