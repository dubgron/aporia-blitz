#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"

#define APORIA_COMMANDLINE_FUNCTION(name) CommandlineResult name(StringList args)

enum class ConsoleWindowState
{
    Closed,
    Opened,
    OpenedFull
};

enum class ConsoleInputState
{
    Empty,
    Typing,
    SelectingSuggestion,
    SelectingFromHistory
};

static char input_buffer[1024];
static i64 selected_command = 0;
static StringList command_hist_help;
static StringList command_history;
static StringList suggestions;
static ConsoleWindowState state = ConsoleWindowState::Closed;
static ConsoleInputState input_state = ConsoleInputState::Empty;

static MemoryArena command_arena;
static MemoryArena suggestion_arena;

static constexpr u64 MAX_SUGGESTIONS = 10;

struct CommandlineResult
{
    i64 return_code = 0;
    String output;
};

typedef APORIA_COMMANDLINE_FUNCTION(commandline_function);

struct CommandlineCommand
{
    String display_name;
    String description;
    commandline_function* func = nullptr;
};

static constexpr u64 MAX_COMMANDS = 1000;
static CommandlineCommand* commands = nullptr;
static u64 command_count = 0;

static void add_command(CommandlineCommand command)
{
    APORIA_ASSERT(command_count < MAX_COMMANDS);
    commands[command_count] = command;
    command_count += 1;
}

static CommandlineCommand* find_command(String command_name)
{
    for (u64 idx = 0; idx < command_count; ++idx)
    {
        if (commands[idx].display_name == command_name)
        {
            return &commands[idx];
        }
    }

    return nullptr;
}

static APORIA_COMMANDLINE_FUNCTION(print_string)
{
    String result = args.join(&command_arena, " ");
    APORIA_LOG(Info, result);

    return CommandlineResult
    {
        .return_code = 0,
        .output = "String printed successfully!"
    };
}

static APORIA_COMMANDLINE_FUNCTION(enable_lights)
{
    if (args.node_count < 1)
    {
        return CommandlineResult
        {
            .return_code = 1,
            .output = "Too few arguments!"
        };
    }

    if (args.first->string == "true" || args.first->string == "1")
    {
        if (!is_lighting_enabled())
        {
            enable_lighting();
            return CommandlineResult
            {
                .return_code = 0,
                .output = "Enabled lighting successfully!"
            };
        }
        else
        {
            return CommandlineResult
            {
                .return_code = 1,
                .output = "Lighting already enabled!"
            };
        }
    }
    else if (args.first->string == "false" || args.first->string == "0")
    {
        if (is_lighting_enabled())
        {
            disable_lighting();
            return CommandlineResult
            {
                .return_code = 0,
                .output = "Disabled lighting successfully!"
            };
        }
        else
        {
            return CommandlineResult
            {
                .return_code = 1,
                .output = "Lighting already disabled!"
            };
        }
    }

    return CommandlineResult
    {
        .return_code = 1,
        .output = "Invalid argument!"
    };
}

struct CommandMatch
{
    String command_name;
    u64 match_score = 0;
};

static StringList find_matching_commands(String command_prefix)
{
    if (command_prefix.length == 0)
    {
        return StringList{};
    }

    ScratchArena temp = scratch_begin();
    defer { scratch_end(temp); };

    CommandMatch* matches = arena_push_uninitialized<CommandMatch>(temp.arena, command_count);
    u64 match_count = 0;

    for (u64 idx = 0; idx < command_count; ++idx)
    {
        const CommandlineCommand& command = commands[idx];
        if (command.display_name.contains(command_prefix))
        {
            matches[match_count] = CommandMatch{ command.display_name, command.display_name.length };
            match_count += 1;
        }
    }

    intro_sort(matches, match_count,
        [](const CommandMatch* m0, const CommandMatch* m1) -> i32
        {
            return m0->match_score - m1->match_score;
        });

    StringList result;
    u64 result_num = min<u64>(MAX_SUGGESTIONS, match_count);
    for (i64 i = 0; i < result_num; ++i)
    {
        result.push_node(&suggestion_arena, matches[i].command_name);
    }

    return result;
}

static void commandline_init()
{
    command_arena = arena_init(MEGABYTES(1));
    suggestion_arena = arena_init(MAX_SUGGESTIONS * sizeof(StringNode));

    commands = arena_push<CommandlineCommand>(&command_arena, MAX_COMMANDS);

    add_command(CommandlineCommand{
        .display_name = "print",
        .description = "Prints strings\nUsage: print string1 [string2...]\n",
        .func = print_string });

    add_command(CommandlineCommand{
        .display_name = "lights.enable",
        .description = "Enable lights\nUsage: lights.enable 0|1|true|false\n",
        .func = enable_lights });
}

static int MyCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        if (input_state == ConsoleInputState::SelectingSuggestion)
        {
            input_state = ConsoleInputState::Typing;
        }
        else if (suggestions.node_count > 0)
        {
            data->DeleteChars(0, data->BufTextLen);
            String comp = suggestions.first->string.substr(data->CursorPos);
            data->InsertChars(0, (char*)comp.data, (char*)(comp.data + comp.length));
        }

        data->InsertChars(data->BufTextLen, " ");
        data->CursorPos = data->BufTextLen;
        data->BufDirty = true;

        suggestions.clear();
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
    {
        // Browsing suggestions
        if ((input_state == ConsoleInputState::Typing || input_state == ConsoleInputState::SelectingSuggestion) && suggestions.node_count > 0)
        {
            input_state = ConsoleInputState::SelectingSuggestion;

            if (selected_command == command_hist_help.node_count)
            {
                selected_command = 0;
            }

            if (data->EventKey == ImGuiKey_UpArrow)
            {
                selected_command = min<i64>(selected_command + 1, suggestions.node_count - 1);
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                selected_command = max<i64>(selected_command - 1, 0);
            }

            StringNode* suggestion_node = suggestions.get_node_at_index(selected_command);
            APORIA_ASSERT(suggestion_node);
            String selected_suggestion = suggestion_node->string;

            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, (char*)(selected_suggestion.data), (char*)(selected_suggestion.data + selected_suggestion.length));
            data->BufDirty = true;
        }
        // Browsing history
        else if (command_hist_help.node_count > 0)
        {
            input_state = ConsoleInputState::SelectingFromHistory;

            if (data->EventKey == ImGuiKey_UpArrow)
            {
                selected_command = max<i64>(selected_command - 1, 0);
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                selected_command = min<i64>(selected_command + 1, command_hist_help.node_count);
            }

            if (selected_command == command_hist_help.node_count)
            {
                data->DeleteChars(0, data->BufTextLen);
                data->BufDirty = true;

                input_state = ConsoleInputState::Empty;
            }
            else
            {
                StringNode* history_node = command_hist_help.get_node_at_index(selected_command);
                APORIA_ASSERT(history_node);
                String selected_history = history_node->string;

                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, (char*)(selected_history.data), (char*)(selected_history.data + selected_history.length));
                data->BufDirty = true;
            }
        }
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
    {
        if (input_state == ConsoleInputState::SelectingSuggestion)
        {
            suggestions.clear();
        }
        input_state = ConsoleInputState::Typing;
        selected_command = 0;
    }

    if (input_state == ConsoleInputState::Typing)
    {
        String current_command = data->Buf;
        suggestions = find_matching_commands(current_command);
    }

    return 0;
}

static void commandline_update()
{
}

static void commandline_draw()
{
    if (input_is_pressed(Key_Tilde))
    {
        if (input_is_held(Key_LShift))
        {
            switch (state)
            {
                case ConsoleWindowState::Closed:        state = ConsoleWindowState::OpenedFull; break;
                case ConsoleWindowState::Opened:        state = ConsoleWindowState::OpenedFull; break;
                case ConsoleWindowState::OpenedFull:    state = ConsoleWindowState::Closed;     break;
            }
        }
        else
        {
            switch (state)
            {
                case ConsoleWindowState::Closed:        state = ConsoleWindowState::Opened;     break;
                case ConsoleWindowState::Opened:        state = ConsoleWindowState::Closed;     break;
                case ConsoleWindowState::OpenedFull:    state = ConsoleWindowState::Opened;     break;
            }
        }
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    f32 target_small = viewport->Size.y * 0.2f;
    f32 target_big = viewport->Size.y * 0.8f;

    constexpr f32 target_closed = 0.f;
    f32 target_open = 0.f;
    if (state == ConsoleWindowState::Closed)
    {
        target_open = target_small;
    }
    else if (state == ConsoleWindowState::Opened)
    {
        target_open = target_small;
    }
    else if (state == ConsoleWindowState::OpenedFull)
    {
        target_open = target_big;
    }

    static f32 current = 0.f;

    f32 target = (state != ConsoleWindowState::Closed) ? target_open : target_closed;
    f32 rate = 1.f / ImGui::GetIO().Framerate;
    current += (target - current) * (1.f - pow(2.f, -20.f * rate));

    if (state == ConsoleWindowState::Closed)
    {
        bool target_closed_reached = abs(current - target_closed) < 0.01f;
        if (target_closed_reached)
        {
            current = target_closed;
            return;
        }
    }

    bool target_open_reached = abs(current - target_open) < 0.01f;
    if (!target_open_reached)
    {
        ImGui::GetIO().ClearInputCharacters();
    }
    else
    {
        current = target_open;
    }

    f32 size_off = 0.f;
    f32 pos_off = 0.f;
    if (current > target_small && current < target_big)
    {
        size_off = current;
        pos_off = 0.f;
    }
    else
    {
        size_off = target_open;
        pos_off = current - target_open;
    }
    ImVec2 console_size = ImVec2(viewport->Size.x, size_off);
    ImVec2 console_pos = ImVec2(viewport->Pos.x, viewport->Pos.y + pos_off);
    ImGui::SetNextWindowPos(console_pos);
    ImGui::SetNextWindowSize(console_size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.8f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("##console", nullptr, flags);
    float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("##command_history", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (command_history.node_count > 0)
        {
            ScratchArena temp = scratch_begin();
            {
                String history = command_history.join(temp.arena, "\n");
                ImGui::TextUnformatted((char*)history.data, (char*)history.data + history.length);
            }
            scratch_end(temp);
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    constexpr ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue
        | ImGuiInputTextFlags_EscapeClearsAll
        | ImGuiInputTextFlags_CallbackCompletion
        | ImGuiInputTextFlags_CallbackHistory
        | ImGuiInputTextFlags_CallbackEdit;
    ImGui::PushItemWidth(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x);
    ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##commandline", input_buffer, IM_ARRAYSIZE(input_buffer), input_text_flags, MyCallback))
    {
        u64 input_length = strlen(input_buffer);
        if (input_length > 0)
        {
            String input_str = push_string(&command_arena, input_buffer).trim();
            StringList input_split = input_str.split(&command_arena, ' ');
            if (input_split.node_count > 0)
            {
                String command_name = input_split.first->string;
                if (CommandlineCommand* command = find_command(command_name))
                {
                    StringList args = input_split.pop_node_front();
                    command->func(args);
                }
            }
            command_history.push_node(&command_arena, input_str);
            if (!command_hist_help.last || command_hist_help.last->string != input_str)
            {
                command_hist_help.push_node(&command_arena, input_str);
            }
            input_buffer[0] = '\0';
            suggestions.clear();
            arena_clear(&suggestion_arena);
            input_state = ConsoleInputState::Empty;
            selected_command = command_hist_help.node_count;
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        input_state = ConsoleInputState::Empty;
        suggestions.clear();
        arena_clear(&suggestion_arena);
    }

    ImGui::PopItemWidth();

    if (suggestions.node_count > 0)
    {
        const ImGuiStyle& style = ImGui::GetStyle();

        ImVec2 region_min = ImGui::GetWindowContentRegionMin();
        ImVec2 region_max = ImGui::GetWindowContentRegionMax();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 suggestions_size = ImVec2(region_max.x - region_min.x, console_size.y * 0.5f);
        ImVec2 suggestions_pos = ImVec2(window_pos.x + region_min.x, window_pos.y + region_max.y - footer_height_to_reserve - suggestions_size.y);
        ImGui::SetNextWindowPos(suggestions_pos);
        ImGui::SetNextWindowBgAlpha(style.Colors[ImGuiCol_PopupBg].w * 0.75f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * 0.75f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
        ImGui::BeginChildFrame(ImGui::GetID("##command_history"), suggestions_size, flags);

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.5f, 0.5f, 1.f));
        u64 idx = suggestions.node_count - 1;
        for (StringNode* node = suggestions.last; node; node = node->prev)
        {
            if (idx == selected_command)
            {
                ImGui::Selectable((char*)node->string.data, true);
                ImGui::SetScrollHereY(0.5f);
            }
            else
            {
                ImGui::Selectable((char*)node->string.data, false);
            }

            idx -= 1;
        }
        ImGui::PopStyleColor();

        ImGui::EndChildFrame();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    ImGui::End();
}

static void commandline_deinit()
{
    arena_deinit(&command_arena);
    arena_deinit(&suggestion_arena);
}
