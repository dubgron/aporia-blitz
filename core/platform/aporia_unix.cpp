#include "aporia_os.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"

namespace Aporia
{
    void* load_library(String library_name)
    {
        ScratchArena temp = scratch_begin();

        String library_full_name = string_concat(temp.arena, library_name, ".so");
        void* library = dlopen(*library_full_name, RTLD_LAZY);

        scratch_end(temp);

        return library;
    }

    bool close_library(void* library_handle)
    {
        return dlclose(library_handle);
    }

    void* load_symbol(void* library_handle, String symbol_name)
    {
        return dlsym(library_handle, *symbol_name);
    }

    String get_last_error()
    {
        return dlerror();
    }

    bool does_directory_exist(String path)
    {
        struct stat st;
        return stat(*path, &st) != -1;
    }

    bool make_directory(String dir_path)
    {
        return mkdir(*dir_path, S_IREAD | S_IWRITE | S_IEXEC) != -1;
    }

    Mutex mutex_create()
    {

        Mutex result;
        pthread_mutex_init((pthread_mutex_t*)result.handle, nullptr);
        return result;
    }

    bool mutex_try_lock(Mutex* mutex)
    {
        i32 return_code = pthread_mutex_trylock((pthread_mutex_t*)mutex->handle);
        return return_code == 0;
    }

    void mutex_lock(Mutex* mutex)
    {
        pthread_mutex_lock((pthread_mutex_t*)mutex->handle);
    }

    void mutex_unlock(Mutex* mutex)
    {
        pthread_mutex_unlock((pthread_mutex_t*)mutex->handle);
    }

    void mutex_destroy(Mutex* mutex)
    {
        pthread_mutex_destroy((pthread_mutex_t*)mutex->handle);
    }

    void watch_project_directory()
    {
        APORIA_LOG(Warning, "This feature is not supported on Unix!");
    }
}
