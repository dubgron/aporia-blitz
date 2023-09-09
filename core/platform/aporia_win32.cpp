#include "aporia_os.hpp"

#include "aporia_assets.hpp"
#include "aporia_debug.hpp"
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
        return GetProcAddress((HMODULE)library_handle, *symbol_name);
    }

    String get_last_error()
    {
        DWORD error_message_id = GetLastError();
        if (error_message_id == 0)
        {
            return String{};
        }

        char buff[256];
        u64 size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_message_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buff, ARRAY_COUNT(buff), NULL);

        String message{ (u8*)buff, size };

        return push_string(&frame_arena, message);
    }

    bool does_directory_exist(String path)
    {
        DWORD attrib = GetFileAttributes(*path);
        return (attrib != INVALID_FILE_ATTRIBUTES) && (attrib & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool make_directory(String path)
    {
        return CreateDirectory(*path, NULL);
    }

    Mutex mutex_create()
    {
        Mutex result;
        result.handle = CreateMutex(0, false, NULL);
        return result;
    }

    bool mutex_try_lock(Mutex* mutex)
    {
        return WaitForSingleObject(mutex->handle, 0) == WAIT_OBJECT_0;
    }

    bool mutex_lock(Mutex* mutex)
    {
        return WaitForSingleObject(mutex->handle, INFINITE) == WAIT_OBJECT_0;
    }

    bool mutex_unlock(Mutex* mutex)
    {
        return ReleaseMutex(mutex->handle);
    }

    bool mutex_destroy(Mutex* mutex)
    {
        return CloseHandle(mutex->handle);
    }
}
