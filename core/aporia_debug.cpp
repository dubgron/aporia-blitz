#include "aporia_debug.hpp"

#include <imgui.h>
#include <stb_sprintf.h>

#include "aporia_utils.hpp"
#include "aporia_window.hpp"

#define STYLE_RESET         0

#define FG_BLACK            30
#define FG_RED              31
#define FG_GREEN            32
#define FG_YELLOW           33
#define FG_BLUE             34
#define FG_MAGENTA          35
#define FG_CYAN             36
#define FG_GRAY             37
#define FG_RESET            39

#define BG_BLACK            40
#define BG_RED              41
#define BG_GREEN            42
#define BG_YELLOW           43
#define BG_BLUE             44
#define BG_MAGENTA          45
#define BG_CYAN             46
#define BG_GRAY             47
#define BG_RESET            49

#define FG_BRIGHT_BLACK     90
#define FG_BRIGHT_RED       91
#define FG_BRIGHT_GREEN     92
#define FG_BRIGHT_YELLOW    93
#define FG_BRIGHT_BLUE      94
#define FG_BRIGHT_MAGENTA   95
#define FG_BRIGHT_CYAN      96
#define FG_BRIGHT_GRAY      97

#define BG_BRIGHT_BLACK     100
#define BG_BRIGHT_RED       101
#define BG_BRIGHT_GREEN     102
#define BG_BRIGHT_YELLOW    103
#define BG_BRIGHT_BLUE      104
#define BG_BRIGHT_MAGENTA   105
#define BG_BRIGHT_CYAN      106
#define BG_BRIGHT_GRAY      107

#define STRINGIFY(x) #x
#define CONSOLE_STYLIZED_FORMAT(style) "\033[" STRINGIFY(style) "m" "%" "\033[0m"

static i64 get_milliseconds()
{
#if defined(APORIA_WINDOWS)
    SYSTEMTIME local_time;
    GetLocalTime(&local_time);
    return (i64)local_time.wMilliseconds;
#elif defined(APORIA_UNIX)
    timeval time;
    gettimeofday(&time, nullptr);
    return (i64)(time.tv_sec * 1000 + time.tv_usec / 1000);
#else
    #error OS not supported!
#endif
}

namespace Aporia
{
    struct LogBuffer
    {
        u8* data = nullptr;
        u64 length = 0;
        u64 max_length = 0;
    };

    static LogBuffer buffer_create(MemoryArena* arena, u64 size)
    {
        LogBuffer result;
        result.data = arena->push<u8>(size);
        result.max_length = size;
        return result;
    }

    static bool will_buffer_overflow_after_append(LogBuffer& buffer, String string)
    {
        return buffer.length + string.length > buffer.max_length;
    }

    static void buffer_add_line(LogBuffer& buffer, String string)
    {
        APORIA_ASSERT(!will_buffer_overflow_after_append(buffer, string));

        memcpy(buffer.data + buffer.length, string.data, string.length);
        buffer.data[buffer.length + string.length] = '\n';
        buffer.length += string.length + 1;
    }

    static void buffer_clear(LogBuffer& buffer)
    {
        buffer.length = 0;
    }

    static LogBuffer console_buffer;
    static LogBuffer file_buffer;

    static String log_name;
    static String log_timestamp;

    static LogLevel min_log_level = Verbose;
    static LogLevel flush_to_console_level = Debug;
    static LogLevel flush_to_file_level = Info;

    static String format_timestamp(MemoryArena* arena, String format)
    {
        time_t time_in_seconds = time(nullptr);
        tm* now_tm = localtime(&time_in_seconds);

        char buff[30];
        u64 length = strftime(buff, ARRAY_COUNT(buff), (const char*)format.data, now_tm);

        String result{ (u8*)buff, length };

        // @NOTE(dubgron): We assume that '%%' in format string means milliseconds. It's dumb, I know...
        if (result.find('%') != String::INVALID_INDEX)
        {
            char temp[4];
            i64 milliseconds = get_milliseconds();
            stbsp_sprintf(temp, "%03lld", milliseconds);

            result = sprintf(arena, result, temp);
        }
        else
        {
            result = push_string(arena, result);
        }

        return result;
    }

    static String to_string(LogLevel level)
    {
        switch (level)
        {
            case Verbose:   return "Verbose";
            case Debug:     return "Debug";
            case Info:      return "Info";
            case Warning:   return "Warning";
            case Error:     return "Error";
            case Critical:  return "Critical";

            default: APORIA_UNREACHABLE(); return String{};
        }
    }

    static String log_level_color_format(LogLevel level)
    {
        switch (level)
        {
            case Verbose:   return CONSOLE_STYLIZED_FORMAT(FG_BRIGHT_BLACK);
            case Debug:     return CONSOLE_STYLIZED_FORMAT(FG_BRIGHT_BLUE);
            case Info:      return CONSOLE_STYLIZED_FORMAT(FG_RESET);
            case Warning:   return CONSOLE_STYLIZED_FORMAT(FG_BRIGHT_YELLOW);
            case Error:     return CONSOLE_STYLIZED_FORMAT(FG_RED);
            case Critical:  return CONSOLE_STYLIZED_FORMAT(BG_RED);

            default: APORIA_UNREACHABLE(); return "%";
        }
    }

    static void flush_logs_to_console()
    {
        if (console_buffer.length > 0)
        {
            printf("%.*s", (i32)console_buffer.length, console_buffer.data);
            buffer_clear(console_buffer);
        }
    }

    static void flush_logs_to_file()
    {
        if (file_buffer.length > 0)
        {
            ScratchArena temp = create_scratch_arena(&persistent_arena);
            {
                String log_filepath = sprintf(temp.arena, "logs/%_latest.log", log_name);

                FILE* log_file = fopen(*log_filepath, "ab");
                APORIA_ASSERT(log_file);

                fwrite(file_buffer.data, file_buffer.length, 1, log_file);
                fclose(log_file);
            }
            rollback_scratch_arena(temp);

            buffer_clear(file_buffer);
        }
    }

    void logging_init(MemoryArena* arena, String name)
    {
        log_name = push_string(arena, name);
        log_timestamp = format_timestamp(arena, "%Y-%m-%d_%H-%M-%S");

        constexpr u64 buffer_size = MEGABYTES(1);
        console_buffer = buffer_create(arena, buffer_size);
        file_buffer = buffer_create(arena, buffer_size);

        ScratchArena temp = create_scratch_arena(&persistent_arena);
        {
            String log_filepath = sprintf(temp.arena, "logs/%_latest.log", log_name);
            FILE* log_file = fopen(*log_filepath, "wb");
            APORIA_ASSERT(log_file);
            fclose(log_file);
        }
        rollback_scratch_arena(temp);
    }

    void logging_deinit()
    {
        flush_logs_to_console();
        flush_logs_to_file();

        ScratchArena temp = create_scratch_arena(&persistent_arena);
        {
            String old_filepath = sprintf(temp.arena, "logs/%_latest.log", log_name);
            String logs = read_file(temp.arena, old_filepath);

            String new_filepath = sprintf(temp.arena, "logs/%_%.log", log_name, log_timestamp);
            FILE* new_logs = fopen(*new_filepath, "wb");
            APORIA_ASSERT(new_logs);

            fwrite(logs.data, logs.length, 1, new_logs);
            fclose(new_logs);
        }
        rollback_scratch_arena(temp);
    }

    void log(String file, i32 line, String function, LogLevel level, String message)
    {
        if (level < min_log_level)
        {
            return;
        }

        ScratchArena temp = create_scratch_arena(&persistent_arena);
        {
            String current_timestamp = format_timestamp(temp.arena, "%H:%M:%S.%%");
            String filename = extract_filename(file);

            String formatted_message = sprintf(temp.arena, "[%] (%@%:%) %: %",
                current_timestamp, filename, function, line, to_string(level), message);

            // Log to console
            {
                String console_message = sprintf(temp.arena, log_level_color_format(level), formatted_message);

                if (will_buffer_overflow_after_append(console_buffer, console_message))
                {
                    flush_logs_to_console();
                }

                buffer_add_line(console_buffer, console_message);

                if (level >= flush_to_console_level)
                {
                    flush_logs_to_console();
                }
            }

            // Log to file
            {
                if (will_buffer_overflow_after_append(file_buffer, formatted_message))
                {
                    flush_logs_to_file();
                }

                buffer_add_line(file_buffer, formatted_message);

                if (level >= flush_to_file_level)
                {
                    flush_logs_to_file();
                }
            }
        }
        rollback_scratch_arena(temp);
    }

    void imgui_init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

#if !defined(APORIA_EMSCRIPTEN)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.f;
            style.Colors[ImGuiCol_WindowBg].w = 1.f;
        }

        ImGui_ImplGlfw_InitForOpenGL(active_window->handle, true);
        ImGui_ImplOpenGL3_Init(OPENGL_SHADER_VERSION);
    }

    void imgui_deinit()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void imgui_frame_begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void imgui_frame_end()
    {
        ImGuiIO& io = ImGui::GetIO();
        GLFWwindow* window = glfwGetCurrentContext();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(window);
        }
    }
}
