#include "aporia_os.hpp"

#include <dlfcn.h>

#include "aporia_game.hpp"

namespace Aporia
{
    void* load_library(String library_name)
    {
        ScratchArena scratch = create_scratch_arena(&persistent_arena);

        const String library_ext = create_string(".so");
        const String library_full_name = library_name.append(scratch.arena, library_ext);
        void* library = dlopen(library_full_name.to_cstring(scratch.arena), RTLD_LAZY);

        rollback_scratch_arena(scratch);

        return library;
    }

    bool close_library(void* library_handle)
    {
        return dlclose(library_handle);
    }

    void* load_symbol(void* library_handle, String symbol_name)
    {
        ScratchArena scratch = create_scratch_arena(&persistent_arena);
        void* symbol = dlsym(library_handle, symbol_name.to_cstring(scratch.arena));
        rollback_scratch_arena(scratch);

        return symbol;
    }

    String get_library_error()
    {
        return create_string(dlerror());
    }
}
