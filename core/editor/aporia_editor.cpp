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
    GIZMO_X_AXIS_INDEX      = INDEX_INVALID - 1,
    GIZMO_Y_AXIS_INDEX      = INDEX_INVALID - 2,
    GIZMO_XY_AXIS_INDEX     = INDEX_INVALID - 3,
};

const Color GIZMO_X_AXIS_COLOR          = Color::Red;
const Color GIZMO_Y_AXIS_COLOR          = Color::Green;
const Color GIZMO_XY_AXIS_COLOR         = Color::Blue;
const Color GIZMO_XY_PLANE_COLOR        = Color{ 0, 0, 255, 85 };
const Color GIZMO_ROTATION_LINE_COLOR   = Color{ 255, 200, 0 };

static i32 gizmo_index = NOTHING_SELECTED_INDEX;
static GizmoType gizmo_type = GizmoType_Translate;
static GizmoSpace gizmo_space = GizmoSpace_World;

static v2 mouse_start_position{ 0.f };
static Entity selected_entity;

enum EditorActionType : u32
{
    EditorActionType_Invalid,
    EditorActionType_SelectEntity,
    EditorActionType_ModifyEntity,
};

struct EditorAction
{
    EditorActionType type = EditorActionType_Invalid;

    EntityID entity_id;
    EntityID prev_entity_id;

    Entity entity;
    Entity prev_entity;
};

// @TODO(dubgron): Use a resizable pool with a free list to make the history as long as it needs to be.
constexpr i64 MAX_EDITOR_ACTIONS_COUNT = 64;
static EditorAction action_history[MAX_EDITOR_ACTIONS_COUNT];
static i64 action_history_begin = 0;
static i64 action_history_end = 0;

#define RING_BUFFER_INDEX_INCREMENT(index) do { index += 1; if (index == MAX_EDITOR_ACTIONS_COUNT) index = 0; } while(0)
#define RING_BUFFER_INDEX_DECREMENT(index) do { index -= 1; if (index == -1) index = MAX_EDITOR_ACTIONS_COUNT - 1; } while(0)

static void editor_select_entity(EntityID new_entity_id)
{
    if (selected_entity_id.index == new_entity_id.index && selected_entity_id.generation == new_entity_id.generation)
        return;

    EditorAction* action = &action_history[action_history_end];
    RING_BUFFER_INDEX_INCREMENT(action_history_end);

    if (action_history_begin == action_history_end)
        RING_BUFFER_INDEX_INCREMENT(action_history_begin);

    action->type = EditorActionType_SelectEntity;
    action->entity_id = new_entity_id;
    action->prev_entity_id = selected_entity_id;

    selected_entity_id = new_entity_id;
}

static void editor_modify_entity(Entity* entity)
{
    EditorAction* action = &action_history[action_history_end];
    RING_BUFFER_INDEX_INCREMENT(action_history_end);

    if (action_history_begin == action_history_end)
        RING_BUFFER_INDEX_INCREMENT(action_history_begin);

    action->type = EditorActionType_ModifyEntity;
    action->entity = *entity;
    action->prev_entity = selected_entity;
}

static void editor_try_undo_last_action()
{
    if (action_history_begin == action_history_end)
        return;

    RING_BUFFER_INDEX_DECREMENT(action_history_end);

    EditorAction last_action = action_history[action_history_end];
    switch (last_action.type)
    {
        case EditorActionType_SelectEntity:
        {
            selected_entity_id = last_action.prev_entity_id;
        }
        break;

        case EditorActionType_ModifyEntity:
        {
            Entity* entity = entity_get(&world, selected_entity_id);
            APORIA_ASSERT(entity);

            *entity = last_action.prev_entity;
        }
        break;
    }
}

static void editor_try_redo_last_action()
{
    EditorAction last_action = action_history[action_history_end];

    if (last_action.type == EditorActionType_Invalid)
        return;

    switch (last_action.type)
    {
        case EditorActionType_SelectEntity:
        {
            selected_entity_id = last_action.entity_id;
        }
        break;

        case EditorActionType_ModifyEntity:
        {
            Entity* entity = entity_get(&world, selected_entity_id);
            APORIA_ASSERT(entity);

            *entity = last_action.entity;
        }
        break;
    }

    RING_BUFFER_INDEX_INCREMENT(action_history_end);
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

    if (ImGui::RadioButton("Translate", gizmo_type == GizmoType_Translate))
    {
        gizmo_type = GizmoType_Translate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", gizmo_type == GizmoType_Rotate))
    {
        gizmo_type = GizmoType_Rotate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", gizmo_type == GizmoType_Scale))
    {
        gizmo_type = GizmoType_Scale;
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
            gizmo_type = GizmoType_Translate;
        }
        else if (input_is_pressed(Key_Num2))
        {
            gizmo_type = GizmoType_Rotate;
        }
        else if (input_is_pressed(Key_Num3))
        {
            gizmo_type = GizmoType_Scale;
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
        else switch (index)
        {
            case GIZMO_X_AXIS_INDEX:
            case GIZMO_Y_AXIS_INDEX:
            case GIZMO_XY_AXIS_INDEX:
            {
                Entity* entity = entity_get(&world, selected_entity_id);
                APORIA_ASSERT(entity);

                gizmo_index = index;
                mouse_start_position = get_mouse_world_position();
                selected_entity = *entity;
            }
        }
    }
    else if (gizmo_index != NOTHING_SELECTED_INDEX)
    {
        Entity* entity = entity_get(&world, selected_entity_id);
        APORIA_ASSERT(entity);

        if (input_is_held(left_mouse_button))
        {
            v2 mouse_current_position = get_mouse_world_position();

            v2 mouse_start_offset = mouse_start_position - selected_entity.position;
            v2 mouse_current_offset = mouse_current_position - selected_entity.position;

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

                    v2 mouse_offset = mouse_current_position - mouse_start_position;

                    switch (index)
                    {
                        case GIZMO_X_AXIS_INDEX:
                        {
                            entity->position = selected_entity.position + glm::dot(mouse_offset, right) * right;
                        }
                        break;

                        case GIZMO_Y_AXIS_INDEX:
                        {
                            entity->position = selected_entity.position + glm::dot(mouse_offset, up) * up;
                        }
                        break;

                        case GIZMO_XY_AXIS_INDEX:
                        {
                            entity->position = selected_entity.position + mouse_offset;
                        }
                        break;
                    }
                }
                break;

                case GizmoType_Rotate:
                {
                    APORIA_ASSERT(index == GIZMO_XY_AXIS_INDEX);

                    f32 base_angle = atan2(mouse_start_offset.y, mouse_start_offset.x);
                    f32 current_angle = atan2(mouse_current_offset.y, mouse_current_offset.x);

                    entity->rotation = selected_entity.rotation + (current_angle - base_angle);
                }
                break;

                case GizmoType_Scale:
                {
                    v2 scale = mouse_current_offset / mouse_start_offset;

                    switch (index)
                    {
                        case GIZMO_X_AXIS_INDEX:
                        {
                            entity->scale.x = selected_entity.scale.x * scale.x;

                            if (input_is_held(Key_LControl))
                            {
                                entity->scale.y = selected_entity.scale.y * scale.x;
                            }

                        }
                        break;

                        case GIZMO_Y_AXIS_INDEX:
                        {
                            entity->scale.y = selected_entity.scale.y * scale.y;

                            if (input_is_held(Key_LControl))
                            {
                                entity->scale.x = selected_entity.scale.x * scale.y;
                            }
                        }
                        break;
                    }
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

    switch (gizmo_type)
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
                // @TODO(dubgron): This should be displayed and work for all gizmos.
                {
                    f32 point_radius = 7.5f;

                    set_editor_index(GIZMO_XY_AXIS_INDEX);
                    draw_circle(start, point_radius, Color::White);
                }

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

                set_editor_index(GIZMO_X_AXIS_INDEX);

                draw_line(start, end, line_thickness, GIZMO_X_AXIS_COLOR);
                v2 p0 = end + end_size * right;
                v2 p1 = end + end_size * up / 3.f;
                v2 p2 = end - end_size * up / 3.f;
                draw_triangle(p0, p1, p2, GIZMO_X_AXIS_COLOR);
            }

            // Draw Y axis
            {
                v2 end = start + line_length * up;

                set_editor_index(GIZMO_Y_AXIS_INDEX);

                draw_line(start, end, line_thickness, GIZMO_Y_AXIS_COLOR);
                v2 p0 = end + end_size * up;
                v2 p1 = end + end_size * right / 3.f;
                v2 p2 = end - end_size * right / 3.f;
                draw_triangle(p0, p1, p2, GIZMO_Y_AXIS_COLOR);
            }

            set_editor_index(NOTHING_SELECTED_INDEX);
        }
        break;

        case GizmoType_Rotate:
        {
            f32 radius = line_length;
            f32 inner_radius = radius - line_thickness;

            set_editor_index(GIZMO_XY_AXIS_INDEX);
            draw_circle(start, radius, inner_radius, GIZMO_XY_AXIS_COLOR);

            if (gizmo_index != NOTHING_SELECTED_INDEX)
            {
                v2 mouse_start_offset = mouse_start_position - selected_entity.position;
                v2 mouse_current_offset = get_mouse_world_position() - selected_entity.position;

                f32 rotation_line_thickness = 3.f;

                v2 dir = glm::normalize(mouse_start_offset);
                v2 end = start + inner_radius * dir;
                draw_line(start, end, rotation_line_thickness, GIZMO_ROTATION_LINE_COLOR);

                dir = glm::normalize(mouse_current_offset);
                end = start + inner_radius * dir;
                draw_line(start, end, rotation_line_thickness, GIZMO_ROTATION_LINE_COLOR);
            }

            set_editor_index(NOTHING_SELECTED_INDEX);
        }
        break;

        case GizmoType_Scale:
        {
            v2 right = camera_rotation * v2{ cos(entity->rotation), sin(entity->rotation) };
            v2 up = v2{ -right.y, right.x };

            v2 end = start + line_length * right;
            f32 box_size = end_size * 0.75f;
            v2 center_offset = (right + up) * box_size / 2.f;

            set_editor_index(GIZMO_X_AXIS_INDEX);

            draw_line(start, end, line_thickness, GIZMO_X_AXIS_COLOR);
            draw_rectangle(end - center_offset, right * box_size, up * box_size, GIZMO_X_AXIS_COLOR);

            end = start + line_length * up;

            set_editor_index(GIZMO_Y_AXIS_INDEX);

            draw_line(start, end, line_thickness, GIZMO_Y_AXIS_COLOR);
            draw_rectangle(end - center_offset, right * box_size, up * box_size, GIZMO_Y_AXIS_COLOR);

            set_editor_index(NOTHING_SELECTED_INDEX);
        }
        break;
    }
}
