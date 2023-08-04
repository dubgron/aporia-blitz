#include "aporia_os.hpp"

#include <windows.h>

#include "aporia_game.hpp"

namespace Aporia
{
    void* load_library(String library_name)
    {
        ScratchArena scratch = create_scratch_arena(&persistent_arena);

        const String library_full_name = library_name.append(scratch.arena, ".dll");
        HMODULE library = LoadLibrary(*library_full_name);

        rollback_scratch_arena(scratch);

        return library;
    }

    bool close_library(void* library_handle)
    {
        return FreeLibrary((HMODULE)library_handle);
    }

    void* load_symbol(void* library_handle, String symbol_name)
    {
        FARPROC symbol = GetProcAddress((HMODULE)library_handle, *symbol_name);
        return symbol;
    }

    String get_library_error()
    {
        DWORD error_message_id = GetLastError();
        if (error_message_id == 0)
        {
            return String{};
        }

        String message_buffer = push_string(&frame_arena, 128);

        size_t size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_message_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer.data, 0, NULL);

        message_buffer.length = size;

        return message_buffer;
    }
}
