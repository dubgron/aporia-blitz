#include "aporia_editor.hpp"

#include "imgui_internal.h"

#include "aporia_camera.hpp"
#include "aporia_game.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"
#include "aporia_world.hpp"

bool editor_is_open = true;

EntityID selected_entity_id;
f32 time_since_selected = 0.f;

enum GizmoType : u8
{
    GizmoType_Translate,
    GizmoType_Rotate,
    GizmoType_Scale,
};

enum GizmoSpace : u8
{
    GizmoSpace_World,
    GizmoSpace_Local,
};

enum EditorIndex : i32
{
    NOTHING_SELECTED_INDEX  = INDEX_INVALID,

    TRANSLATE_X_AXIS_INDEX  = INDEX_INVALID - 1,
    TRANSLATE_Y_AXIS_INDEX  = INDEX_INVALID - 2,
    TRANSLATE_XY_AXIS_INDEX = INDEX_INVALID - 3,

    ROTATE_INDEX            = INDEX_INVALID - 4,

    SCALE_X_AXIS_INDEX      = INDEX_INVALID - 5,
    SCALE_Y_AXIS_INDEX      = INDEX_INVALID - 6,
};

const Color GIZMO_X_AXIS_COLOR          = Color::Red;
const Color GIZMO_Y_AXIS_COLOR          = Color::Green;
const Color GIZMO_XY_AXIS_COLOR         = Color::Blue;
const Color GIZMO_XY_PLANE_COLOR        = Color{ 0, 0, 255, 85 };
const Color GIZMO_ROTATION_LINE_COLOR   = Color{ 255, 200, 0 };

static i32 gizmo_index = NOTHING_SELECTED_INDEX;
static GizmoType displayed_gizmo_type = GizmoType_Translate;
static GizmoSpace gizmo_space = GizmoSpace_World;

static v2 mouse_initial_position{ 0.f };
static Entity entity_initial_state;

enum EditorActionType : u32
{
    EditorActionType_Invalid,
    EditorActionType_SelectEntity,
    EditorActionType_ModifyEntity,
};

struct EditorAction
{
    EditorActionType type = EditorActionType_Invalid;

    Entity entity_state;
};

// @TODO(dubgron): Use a resizable array to make the history as long as it needs to be.
constexpr i64 MAX_EDITOR_ACTIONS_COUNT = 256;
static EditorAction action_history[MAX_EDITOR_ACTIONS_COUNT];
static i64 action_history_cursor = 0;
static i64 action_history_count = 0;
static i64 action_history_redos = 0;

static EditorAction* editor_make_new_action()
{
    EditorAction* action = &action_history[action_history_cursor];
    ring_buffer_increment_index(&action_history_cursor, MAX_EDITOR_ACTIONS_COUNT);

    action_history_count += 1;
    if (action_history_count > MAX_EDITOR_ACTIONS_COUNT)
        action_history_count = MAX_EDITOR_ACTIONS_COUNT;

    action_history_redos = 0;

    return action;
}

static void editor_select_entity(EntityID new_entity_id)
{
    if (selected_entity_id.index == new_entity_id.index && selected_entity_id.generation == new_entity_id.generation)
        return;

    EditorAction* action = editor_make_new_action();
    action->type = EditorActionType_SelectEntity;

    selected_entity_id = new_entity_id;
    if (selected_entity_id.index != INDEX_INVALID)
    {
        action->entity_state = *entity_get(&world, selected_entity_id);
    }
}

static void editor_modify_entity(Entity* entity)
{
    EditorAction* action = editor_make_new_action();
    action->type = EditorActionType_ModifyEntity;
    action->entity_state = *entity;
}

static void editor_try_undo_last_action()
{
    if (action_history_count == 0)
        return;

    ring_buffer_decrement_index(&action_history_cursor, MAX_EDITOR_ACTIONS_COUNT);
    action_history_count -= 1;
    action_history_redos += 1;

    Entity* prev_entity_state = nullptr;
    if (action_history_count == 0)
    {
        static Entity default_entity;
        prev_entity_state = &default_entity;
    }
    else
    {
        i64 prev_cursor = action_history_cursor;
        ring_buffer_decrement_index(&prev_cursor, MAX_EDITOR_ACTIONS_COUNT);

        prev_entity_state = &action_history[prev_cursor].entity_state;
    }

    EditorAction* last_action = &action_history[action_history_cursor];
    switch (last_action->type)
    {
        case EditorActionType_SelectEntity:
        {
            selected_entity_id = prev_entity_state->id;
        }
        break;

        case EditorActionType_ModifyEntity:
        {
            Entity* entity = entity_get(&world, selected_entity_id);
            APORIA_ASSERT(entity);

            *entity = *prev_entity_state;
        }
        break;
    }
}

static void editor_try_redo_last_action()
{
    if (action_history_redos == 0)
        return;

    EditorAction last_action = action_history[action_history_cursor];

    ring_buffer_increment_index(&action_history_cursor, MAX_EDITOR_ACTIONS_COUNT);
    action_history_count += 1;
    action_history_redos -= 1;

    switch (last_action.type)
    {
        case EditorActionType_SelectEntity:
        {
            selected_entity_id = last_action.entity_state.id;
        }
        break;

        case EditorActionType_ModifyEntity:
        {
            Entity* entity = entity_get(&world, selected_entity_id);
            APORIA_ASSERT(entity);

            *entity = last_action.entity_state;
        }
        break;
    }
}

void editor_update(f32 frame_time)
{
    input_set_active_owner(InputOwner_Editor);

    if (input_is_pressed(Key_F1))
        editor_is_open = !editor_is_open;

    if (!editor_is_open)
        return;

    time_since_selected += frame_time;

    // @TODO(dubgron): This feels weird, there's probably a better solution.
    for (i32 idx = 0; idx < world.entity_count; ++idx)
    {
        entity_flags_set(&world.entity_array[idx], EntityFlag_SkipFrameInterpolation);
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(viewport);

    ImGui::Begin("Tools");

    if (ImGui::RadioButton("Translate", displayed_gizmo_type == GizmoType_Translate))
    {
        displayed_gizmo_type = GizmoType_Translate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", displayed_gizmo_type == GizmoType_Rotate))
    {
        displayed_gizmo_type = GizmoType_Rotate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", displayed_gizmo_type == GizmoType_Scale))
    {
        displayed_gizmo_type = GizmoType_Scale;
    }

    if (ImGui::RadioButton("World", gizmo_space == GizmoSpace_World))
    {
        gizmo_space = GizmoSpace_World;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", gizmo_space == GizmoSpace_Local))
    {
        gizmo_space = GizmoSpace_Local;
    }

    ImGui::Separator();

    if (ImGui::Button("Play (F1)"))
    {
        editor_is_open = !editor_is_open;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset camera"))
    {
        active_camera->view = CameraView{};
        active_camera->projection = CameraProjection{};
        active_camera->apply_config();
        active_camera->mark_as_dirty(CameraDirtyFlag_View | CameraDirtyFlag_Projection);
    }

    ImGui::End();

    static const ImGuiID viewport_id = ImHashStr("Viewport");

    ImGuiContext* context = ImGui::GetCurrentContext();
    bool mouse_within_viewport = context->HoveredWindow && (context->HoveredWindow->ID == viewport_id);
    bool focused_on_viewport = context->NavWindow && (context->NavWindow->ID == viewport_id);

    if (focused_on_viewport)
    {
        active_camera->control_movement(frame_time);
        active_camera->control_rotation(frame_time);
        active_camera->control_zoom(frame_time);
    }

    InputState left_mouse_button = input_get(Mouse_Button1);

    if (!input_is_held(left_mouse_button))
    {
        if (input_is_pressed(Key_Num1))
        {
            displayed_gizmo_type = GizmoType_Translate;
        }
        else if (input_is_pressed(Key_Num2))
        {
            displayed_gizmo_type = GizmoType_Rotate;
        }
        else if (input_is_pressed(Key_Num3))
        {
            displayed_gizmo_type = GizmoType_Scale;
        }

        if (input_is_pressed(Key_F2))
        {
            switch (gizmo_space)
            {
                case GizmoSpace_World: gizmo_space = GizmoSpace_Local; break;
                case GizmoSpace_Local: gizmo_space = GizmoSpace_World; break;
            }
        }

        if (input_is_held(Key_LControl))
        {
            InputState key_z = input_get(Key_Z);
            if (input_is_pressed(key_z) || input_is_repeated(key_z))
            {
                editor_try_undo_last_action();
            }

            InputState key_y = input_get(Key_Y);
            if (input_is_pressed(key_y) || input_is_repeated(key_y))
            {
                editor_try_redo_last_action();
            }
        }
    }

    i32 index = gizmo_index;
    if (gizmo_index == NOTHING_SELECTED_INDEX && mouse_within_viewport)
    {
        index = read_editor_index();
    }

    if (mouse_within_viewport && input_is_pressed(left_mouse_button))
    {
        if (index > NOTHING_SELECTED_INDEX)
        {
            i32 generation = world.entity_array[index].id.generation;
            EntityID new_entity_id = EntityID{ index, generation };
            editor_select_entity(new_entity_id);

            time_since_selected = 0.f;
        }
        else if (index == NOTHING_SELECTED_INDEX)
        {
            editor_select_entity(EntityID{});
        }
        else
        {
            Entity* entity = entity_get(&world, selected_entity_id);
            APORIA_ASSERT(entity);

            gizmo_index = index;
            mouse_initial_position = get_mouse_world_position();
            entity_initial_state = *entity;
        }
    }
    else if (gizmo_index != NOTHING_SELECTED_INDEX)
    {
        Entity* entity = entity_get(&world, selected_entity_id);
        APORIA_ASSERT(entity);

        GizmoType gizmo_type;
        switch (gizmo_index)
        {
            case TRANSLATE_X_AXIS_INDEX:
            case TRANSLATE_Y_AXIS_INDEX:
            case TRANSLATE_XY_AXIS_INDEX:
            {
                gizmo_type = GizmoType_Translate;
            }
            break;

            case ROTATE_INDEX:
            {
                gizmo_type = GizmoType_Rotate;
            }
            break;

            case SCALE_X_AXIS_INDEX:
            case SCALE_Y_AXIS_INDEX:
            {
                gizmo_type = GizmoType_Scale;
            }
            break;
        }

        if (input_is_held(left_mouse_button))
        {
            v2 mouse_current_position = get_mouse_world_position();

            v2 mouse_start_offset = mouse_initial_position - entity_initial_state.position;
            v2 mouse_current_offset = mouse_current_position - entity_initial_state.position;

            switch (gizmo_type)
            {
                case GizmoType_Translate:
                {
                    v2 right = v2{ 1.f, 0.f };
                    v2 up = v2{ 0.f, 1.f };

                    if (gizmo_space == GizmoSpace_Local)
                    {
                        right = v2{ cos(entity->rotation), sin(entity->rotation) };
                        up = v2{ -right.y, right.x };
                    }

                    v2 mouse_offset = mouse_current_position - mouse_initial_position;

                    switch (gizmo_index)
                    {
                        case TRANSLATE_X_AXIS_INDEX:
                        {
                            entity->position = entity_initial_state.position + glm::dot(mouse_offset, right) * right;
                        }
                        break;

                        case TRANSLATE_Y_AXIS_INDEX:
                        {
                            entity->position = entity_initial_state.position + glm::dot(mouse_offset, up) * up;
                        }
                        break;

                        case TRANSLATE_XY_AXIS_INDEX:
                        {
                            entity->position = entity_initial_state.position + mouse_offset;
                        }
                        break;
                    }
                }
                break;

                case GizmoType_Rotate:
                {
                    f32 base_angle = atan2(mouse_start_offset.y, mouse_start_offset.x);
                    f32 current_angle = atan2(mouse_current_offset.y, mouse_current_offset.x);

                    entity->rotation = entity_initial_state.rotation + (current_angle - base_angle);
                }
                break;

                case GizmoType_Scale:
                {
                    v2 scale{ 1.f };

                    v2 right = v2{ cos(entity->rotation), sin(entity->rotation) };
                    v2 up = v2{ -right.y, right.x };

                    switch (gizmo_index)
                    {
                        case SCALE_X_AXIS_INDEX:
                        {
                            scale.x = glm::dot(mouse_current_offset, right) / glm::dot(mouse_start_offset, right);

                            if (input_is_held(Key_LControl))
                                scale.y = scale.x;
                        }
                        break;

                        case SCALE_Y_AXIS_INDEX:
                        {
                            scale.y = glm::dot(mouse_current_offset, up) / glm::dot(mouse_start_offset, up);

                            if (input_is_held(Key_LControl))
                                scale.x = scale.y;
                        }
                        break;
                    }

                    entity->scale = entity_initial_state.scale * scale;
                }
                break;
            }
        }
        else if (input_is_released(left_mouse_button))
        {
            switch (gizmo_type)
            {
                case GizmoType_Translate:
                case GizmoType_Rotate:
                case GizmoType_Scale:
                {
                    editor_modify_entity(entity);
                }
            }

            gizmo_index = NOTHING_SELECTED_INDEX;
        }
    }
}

void editor_draw_selected_entity()
{
    Entity* entity = entity_get(&world, selected_entity_id);
    if (!entity)
        return;

    // @HACK(dubgron): It's stupid and there should be an easier way to do it.
    u32 shader = entity->shader_id;
    f32 z = entity->z;
    entity->shader_id = editor_selected_shader;
    entity->z = 0.99f;
    draw_entity(*entity);
    entity->shader_id = shader;
    entity->z = z;
}

void editor_draw_gizmos()
{
    Entity* entity = entity_get(&world, selected_entity_id);
    if (!entity)
        return;

    f32 line_thickness = 5.f;
    f32 line_length = 100.f;
    f32 end_size = 25.f;

    const m4& world_to_clip = active_camera->calculate_view_projection_matrix();

    // @NOTE(dubgron): Precalculated following lines:
    //     viewport_to_clip = glm::scale(glm::mat4{ 1.f }, glm::vec3{ width / 2.f, height / 2.f, -1.f });
    //     viewport_to_clip = glm::translate(viewport_to_clip, glm::vec3{ width / 2.f, height / 2.f, 0.f });
    m4 clip_to_viewport{
        viewport_width / 2.f, 0.f, 0.f, 0.f,
        0.f, viewport_height / 2.f, 0.f, 0.f,
        0.f, 0.f, -1.f, 0.f,
        viewport_width / 2.f, viewport_height / 2.f, 0.f, 1.f };

    v2 start = clip_to_viewport * world_to_clip * v4{ entity->position, 0.f, 1.f };

    m2 camera_rotation = active_camera->view.matrix;

    f32 point_radius = 7.5f;
    set_editor_index(TRANSLATE_XY_AXIS_INDEX);
    draw_circle(start, point_radius, Color::White);

    switch (displayed_gizmo_type)
    {
        case GizmoType_Translate:
        {
            v2 right = v2{ 1.f, 0.f };

            if (gizmo_space == GizmoSpace_Local)
                right = v2{ cos(entity->rotation), sin(entity->rotation) };

            right = camera_rotation * right;
            v2 up = v2{ -right.y, right.x };

            // Draw XY axis
            {
                v2 end = start + line_length * (right + up);

                f32 xy_plane_size = end_size;
                v2 xy_plane_right = xy_plane_size * right;
                v2 xy_plane_up = xy_plane_size * up;

                draw_rectangle(start, xy_plane_right, xy_plane_up, GIZMO_XY_PLANE_COLOR);

                draw_line(start + xy_plane_right, start + xy_plane_right + xy_plane_up, line_thickness / 2.f, GIZMO_X_AXIS_COLOR);
                draw_line(start + xy_plane_up, start + xy_plane_right + xy_plane_up, line_thickness / 2.f, GIZMO_Y_AXIS_COLOR);
            }

            // Draw X axis
            {
                v2 end = start + line_length * right;

                set_editor_index(TRANSLATE_X_AXIS_INDEX);

                draw_line(start, end, line_thickness, GIZMO_X_AXIS_COLOR);
                v2 p0 = end + end_size * right;
                v2 p1 = end + end_size * up / 3.f;
                v2 p2 = end - end_size * up / 3.f;
                draw_triangle(p0, p1, p2, GIZMO_X_AXIS_COLOR);
            }

            // Draw Y axis
            {
                v2 end = start + line_length * up;

                set_editor_index(TRANSLATE_Y_AXIS_INDEX);

                draw_line(start, end, line_thickness, GIZMO_Y_AXIS_COLOR);
                v2 p0 = end + end_size * up;
                v2 p1 = end + end_size * right / 3.f;
                v2 p2 = end - end_size * right / 3.f;
                draw_triangle(p0, p1, p2, GIZMO_Y_AXIS_COLOR);
            }
        }
        break;

        case GizmoType_Rotate:
        {
            f32 radius = line_length;
            f32 inner_radius = radius - line_thickness;

            set_editor_index(ROTATE_INDEX);
            draw_circle(start, radius, inner_radius, GIZMO_XY_AXIS_COLOR);

            if (gizmo_index == ROTATE_INDEX)
            {
                v2 mouse_start_offset = mouse_initial_position - entity_initial_state.position;
                v2 mouse_current_offset = get_mouse_world_position() - entity_initial_state.position;

                f32 rotation_line_thickness = 3.f;

                v2 dir = glm::normalize(mouse_start_offset);
                v2 end = start + inner_radius * dir;
                draw_line(start, end, rotation_line_thickness, GIZMO_ROTATION_LINE_COLOR);

                dir = glm::normalize(mouse_current_offset);
                end = start + inner_radius * dir;
                draw_line(start, end, rotation_line_thickness, GIZMO_ROTATION_LINE_COLOR);
            }
        }
        break;

        case GizmoType_Scale:
        {
            v2 right = camera_rotation * v2{ cos(entity->rotation), sin(entity->rotation) };
            v2 up = v2{ -right.y, right.x };

            v2 end = start + line_length * right;
            f32 box_size = end_size * 0.75f;
            v2 center_offset = (right + up) * box_size / 2.f;

            set_editor_index(SCALE_X_AXIS_INDEX);

            draw_line(start, end, line_thickness, GIZMO_X_AXIS_COLOR);
            draw_rectangle(end - center_offset, right * box_size, up * box_size, GIZMO_X_AXIS_COLOR);

            end = start + line_length * up;

            set_editor_index(SCALE_Y_AXIS_INDEX);

            draw_line(start, end, line_thickness, GIZMO_Y_AXIS_COLOR);
            draw_rectangle(end - center_offset, right * box_size, up * box_size, GIZMO_Y_AXIS_COLOR);
        }
        break;
    }

    set_editor_index(NOTHING_SELECTED_INDEX);
}
