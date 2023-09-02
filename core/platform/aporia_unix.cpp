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
}
