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
elseif (WIN32)
    message(STATUS "Using Win32 for window creation")
    target_compile_definitions(glfw PUBLIC _GLFW_WIN32)
elseif (UNIX)
    message(STATUS "Using X11 for window creation")
    target_compile_definitions(glfw PUBLIC _GLFW_X11)

    find_package(X11 REQUIRED)

    # Set up library and include paths
    list(APPEND glfw_INCLUDE_DIRS "${X11_X11_INCLUDE_PATH}")

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_INCLUDE_PATH)
        message(FATAL_ERROR "RandR headers not found; install libxrandr development package")
    endif()

    # Check for Xinerama (legacy multi-monitor support)
    if (NOT X11_Xinerama_INCLUDE_PATH)
        message(FATAL_ERROR "Xinerama headers not found; install libxinerama development package")
    endif()

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_INCLUDE_PATH)
        message(FATAL_ERROR "XKB headers not found; install X11 development package")
    endif()

    # Check for Xcursor (cursor creation from RGBA images)
    if (NOT X11_Xcursor_INCLUDE_PATH)
        message(FATAL_ERROR "Xcursor headers not found; install libxcursor development package")
    endif()

    # Check for XInput (modern HID input)
    if (NOT X11_Xi_INCLUDE_PATH)
        message(FATAL_ERROR "XInput headers not found; install libxi development package")
    endif()

    # Check for X Shape (custom window input shape)
    if (NOT X11_Xshape_INCLUDE_PATH)
        message(FATAL_ERROR "X Shape headers not found; install libxext development package")
    endif()
endif()

if (UNIX AND NOT APPLE)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(glfw PUBLIC rt)
    endif()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(glfw PUBLIC m)
    endif()

    if (CMAKE_DL_LIBS)
        target_link_libraries(glfw PUBLIC dl)
    endif()

    target_link_libraries(glfw PUBLIC pthread)
endif()

if (APPLE)
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
        target_sources(glfw PRIVATE
            ${GLFW_SOURCE_DIR}/linux_joystick.h
            ${GLFW_SOURCE_DIR}/linux_joystick.c)
    else()
        target_sources(glfw PRIVATE
            ${GLFW_SOURCE_DIR}/null_joystick.h
            ${GLFW_SOURCE_DIR}/null_joystick.c)
    endif()
endif()

target_include_directories(glfw PUBLIC ${GLFW_INCLUDE_DIR})
