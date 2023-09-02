#include "aporia_os.hpp"

#include "aporia_game.hpp"

namespace Aporia
{
    void* load_library(String library_name)
    {
        ScratchArena scratch = create_scratch_arena(&persistent_arena);

        const String library_full_name = library_name.append(scratch.arena, ".so");
        void* library = dlopen(*library_full_name, RTLD_LAZY);

        rollback_scratch_arena(scratch);

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
        i32 return_code = stat(*path, &st);
        return return_code != -1;
    }

    bool make_directory(String dir_path)
    {
        i32 return_code = mkdir(*dir_path, S_IREAD | S_IWRITE | S_IEXEC);
        return return_code != -1;
    }
}
