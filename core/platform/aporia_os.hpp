#pragma once

#include "aporia_strings.hpp"
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
        void* handle = nullptr;
    };

    Mutex mutex_create();
    bool mutex_try_lock(Mutex* mutex);
    bool mutex_lock(Mutex* mutex);
    bool mutex_unlock(Mutex* mutex);
    bool mutex_destroy(Mutex* mutex);
}
