#pragma once

#include "aporia_string.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    void* load_library(String library_name);
    bool close_library(void* library_handle);

    void* load_symbol(void* library_handle, String symbol_name);

    String get_last_error();

    bool does_directory_exist(String path);
    bool make_directory(String path);

    struct Mutex
    {
        // @NOTE(dubgron): The size of the handle is selected so it can hold the mutex
        // on every supported system.
        //
        // On Windows,   sizeof(CRITICAL_SECTION) == 40
        // On Linux,     sizeof(pthread_mutex_t) == 40
        // On MacOs,     sizeof(pthread_mutex_t) == 80
        //
        // It's suboptimal, but we don't store a lot of mutexes and it frees us from
        // dynamically allocating the needed memory.
        u8 handle[80] = {};
    };

#if defined(APORIA_WINDOWS)
        static_assert(sizeof(CRITICAL_SECTION) <= sizeof(Mutex));
#elif defined(APORIA_UNIX)
        static_assert(sizeof(pthread_mutex_t) <= sizeof(Mutex));
#endif

    Mutex mutex_create();
    bool mutex_try_lock(Mutex* mutex);
    void mutex_lock(Mutex* mutex);
    void mutex_unlock(Mutex* mutex);
    void mutex_destroy(Mutex* mutex);

    void watch_project_directory();
}
