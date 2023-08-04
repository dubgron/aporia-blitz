#include "aporia_debug.hpp"

#include <chrono>

#include <imgui.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "aporia_game.hpp"
#include "aporia_window.hpp"

#if defined(__cpp_lib_format)
    #include <format>
#else
    #include <ctime>
    // Workaround for compilers which don't support std::format
    std::string format_timestamp(Aporia::String fmt, const std::chrono::system_clock::time_point& timestamp)
    {
        const std::time_t now_c = std::chrono::system_clock::to_time_t(timestamp);
        const std::tm now_tm = *std::localtime(&now_c);

        char buff[30];
        strftime(buff, sizeof(buff), (const char*)fmt.data, &now_tm);

        return buff;
    }
#endif

namespace Aporia
{
    std::shared_ptr<spdlog::logger> logger;

    void logging_init(String name)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);

#if defined(__cpp_lib_format)
        static const std::chrono::zoned_time timestamp{ std::chrono::current_zone(), std::chrono::system_clock::now() };
        static const std::string logfile = std::format("logs/{0:%Y-%m-%d_%H-%M-%OS}.log", timestamp);
#else
        static const std::string logfile = format_timestamp("logs/%Y-%m-%d_%H-%M-%OS.log", std::chrono::system_clock::now());
#endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile);

        name = push_string(temp.arena, name);

        String logger_pattern_prefix = "[%Y-%m-%d %H:%M:%S.%e] [";
        String logger_pattern_sufix = "] [%^%l%$] (%s:%!@%#) : %v";

        StringList logger_pattern_list;
        logger_pattern_list.push_node(temp.arena, logger_pattern_prefix);
        logger_pattern_list.push_node(temp.arena, name);
        logger_pattern_list.push_node(temp.arena, logger_pattern_sufix);

        String logger_pattern = logger_pattern_list.join(temp.arena);

        logger = std::make_shared<spdlog::logger>(std::string{ name }, spdlog::sinks_init_list{ std::move(console_sink), std::move(file_sink) });
        logger->set_pattern(std::string{ logger_pattern });
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);

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
