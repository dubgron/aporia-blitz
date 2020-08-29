set(GLFW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glfw/include")
set(GLFW_SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glfw/src")

add_library(glfw STATIC
    ${GLFW_SOURCE_DIR}/internal.h
    ${GLFW_SOURCE_DIR}/mappings.h
    ${GLFW_SOURCE_DIR}/context.c
    ${GLFW_SOURCE_DIR}/init.c
    ${GLFW_SOURCE_DIR}/input.c
    ${GLFW_SOURCE_DIR}/monitor.c
    ${GLFW_SOURCE_DIR}/vulkan.c
    ${GLFW_SOURCE_DIR}/window.c)

if (APPLE)
    message(STATUS "Using Cocoa for window creation")
    target_compile_definitions(glfw PUBLIC _GLFW_COCOA)
    target_sources(glfw PRIVATE
        ${GLFW_SOURCE_DIR}/cocoa_platform.h
        ${GLFW_SOURCE_DIR}/cocoa_joystick.h
        ${GLFW_SOURCE_DIR}/posix_thread.h
        ${GLFW_SOURCE_DIR}/nsgl_context.h
        ${GLFW_SOURCE_DIR}/egl_context.h
        ${GLFW_SOURCE_DIR}/osmesa_context.h
        ${GLFW_SOURCE_DIR}/cocoa_init.m
        ${GLFW_SOURCE_DIR}/cocoa_joystick.m
        ${GLFW_SOURCE_DIR}/cocoa_monitor.m
        ${GLFW_SOURCE_DIR}/cocoa_window.m
        ${GLFW_SOURCE_DIR}/cocoa_time.c
        ${GLFW_SOURCE_DIR}/posix_thread.c
        ${GLFW_SOURCE_DIR}/nsgl_context.m
        ${GLFW_SOURCE_DIR}/egl_context.c
        ${GLFW_SOURCE_DIR}/osmesa_context.c)
elseif (WIN32)
    message(STATUS "Using Win32 for window creation")
    target_compile_definitions(glfw PUBLIC _GLFW_WIN32)
    target_sources(glfw PRIVATE
        ${GLFW_SOURCE_DIR}/win32_platform.h
        ${GLFW_SOURCE_DIR}/win32_joystick.h
        ${GLFW_SOURCE_DIR}/wgl_context.h
        ${GLFW_SOURCE_DIR}/egl_context.h
        ${GLFW_SOURCE_DIR}/osmesa_context.h
        ${GLFW_SOURCE_DIR}/win32_init.c
        ${GLFW_SOURCE_DIR}/win32_joystick.c
        ${GLFW_SOURCE_DIR}/win32_monitor.c
        ${GLFW_SOURCE_DIR}/win32_time.c
        ${GLFW_SOURCE_DIR}/win32_thread.c
        ${GLFW_SOURCE_DIR}/win32_window.c
        ${GLFW_SOURCE_DIR}/wgl_context.c
        ${GLFW_SOURCE_DIR}/egl_context.c
        ${GLFW_SOURCE_DIR}/osmesa_context.c)
elseif (UNIX)
    message(STATUS "Using X11 for window creation")
    target_compile_definitions(glfw PUBLIC _GLFW_X11)
    target_sources(glfw PRIVATE
        ${GLFW_SOURCE_DIR}/x11_platform.h
        ${GLFW_SOURCE_DIR}/xkb_unicode.h
        ${GLFW_SOURCE_DIR}/posix_time.h
        ${GLFW_SOURCE_DIR}/posix_thread.h
        ${GLFW_SOURCE_DIR}/glx_context.h
        ${GLFW_SOURCE_DIR}/egl_context.h
        ${GLFW_SOURCE_DIR}/osmesa_context.h
        ${GLFW_SOURCE_DIR}/x11_init.c
        ${GLFW_SOURCE_DIR}/x11_monitor.c
        ${GLFW_SOURCE_DIR}/x11_window.c
        ${GLFW_SOURCE_DIR}/xkb_unicode.c
        ${GLFW_SOURCE_DIR}/posix_time.c
        ${GLFW_SOURCE_DIR}/posix_thread.c
        ${GLFW_SOURCE_DIR}/glx_context.c
        ${GLFW_SOURCE_DIR}/egl_context.c
        ${GLFW_SOURCE_DIR}/osmesa_context.c)
else()
    message(FATAL_ERROR "No supported platform was detected")
endif()

if (UNIX)
    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        target_sources(glfw PRIVATE linux_joystick.h linux_joystick.c)
    else()
        target_sources(glfw PRIVATE null_joystick.h null_joystick.c)
    endif()
endif()

target_include_directories(glfw PUBLIC ${GLFW_INCLUDE_DIR})
