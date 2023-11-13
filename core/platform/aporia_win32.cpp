#include "aporia_os.hpp"

#include "aporia_assets.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"

namespace Aporia
{
    void* load_library(String library_name)
    {
        ScratchArena temp = get_scratch_arena();

        const String library_full_name = library_name.append(temp.arena, ".dll");
        HMODULE library = LoadLibrary(*library_full_name);

        release_scratch_arena(temp);

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
            NULL, error_message_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buff, sizeof(buff), NULL);

        String message{ (u8*)buff, size };

        return push_string(&memory.frame, message);
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

    static DWORD internal_watch_project_directory(void* data)
    {
        // @NOTE(dubgron): Initially I intended to use SHChangeNotifyRegister as
        // it seemed to be more robust, but it turned out that in order to capture
        // the user message it sends, we'd need to have an access to the window
        // procedure's message dispatch. Because we use GLFW, which has their own
        // window procedure, we would have to modify its source_file code. Therefore
        // it's easier to use ReadDirectoryChangesW.

        HANDLE directory_handle = CreateFile(".", FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (directory_handle == INVALID_HANDLE_VALUE)
        {
            printf("%s\n", *get_last_error());
            return 1;
        }

        constexpr u64 size = KILOBYTES(1);
        FILE_NOTIFY_INFORMATION file_notifies[size];

        for (;;)
        {
            memset(file_notifies, 0, size);
            DWORD bytes_returned;

            bool success = ReadDirectoryChangesW(directory_handle, file_notifies, size, true,
                FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes_returned, NULL, NULL);

            if (!success || bytes_returned == 0)
            {
                printf("%s\n", *get_last_error());
                continue;
            }

            u64 idx = 0;
            while (idx < bytes_returned)
            {
                char buff[256];
                u64 size = WideCharToMultiByte(CP_UTF8, 0, file_notifies[idx].FileName,
                    file_notifies[idx].FileNameLength / sizeof(WCHAR), buff, sizeof(buff), NULL, NULL);

                String changed_file{ (u8*)buff, size };
                fix_path_slashes(&changed_file);

                if (Asset* changed_asset = get_asset_by_source_file(changed_file))
                {
                    asset_change_status(changed_asset, AssetStatus::NeedsReload);
                }

                if (file_notifies[idx].NextEntryOffset == 0)
                {
                    break;
                }

                idx += file_notifies[idx].NextEntryOffset;
            }
        }

        return 0;
    }

    void watch_project_directory()
    {
        DWORD thread_id;
        HANDLE thread_handle = CreateThread(NULL, 0, internal_watch_project_directory, NULL, 0, &thread_id);
        CloseHandle(thread_handle);
    }
}
