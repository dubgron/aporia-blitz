set(IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/imgui")

add_library(imgui STATIC
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_opengl3.cpp)

target_link_libraries(imgui glfw)

if (NOT APORIA_EMSCRIPTEN)
    target_link_libraries(imgui gl3w)
endif()

# Specified the C++ standard for AppleClang
target_compile_features(imgui PUBLIC cxx_std_11)

target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
