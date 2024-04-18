#include "aporia_editor.hpp"

#include "aporia_camera.hpp"
#include "aporia_game.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"
#include "aporia_world.hpp"

bool editor_is_open = true;
f32 time_since_selected = 0.f;

static bool editor_is_mouse_within_viewport = false;

static EntityID selected_entity;

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

static GizmoType gizmo_type = GizmoType_Translate;
static GizmoSpace gizmo_space = GizmoSpace_World;

static v2 mouse_start_position{ 0.f };

// Entity start transform
static v2 entity_start_position{ 0.f };
static f32 entity_start_rotation = 0.f;
static v2 entity_start_scale{ 0.f };

static i32 gizmo_in_use_index = NOTHING_SELECTED_INDEX;

void editor_update(f32 frame_time)
{
    if (input_is_pressed(Key_F1))
        editor_is_open = !editor_is_open;

    if (!editor_is_open)
    {
        editor_is_mouse_within_viewport = false;
        return;
    }

    active_camera->control_movement(frame_time);
    active_camera->control_rotation(frame_time);
    active_camera->control_zoom(frame_time);

    if (!input_is_held(Mouse_Button1))
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

        if (input_is_pressed(Key_F1))
        {
            gizmo_space = GizmoSpace_World;
        }
        else if (input_is_pressed(Key_F2))
        {
            gizmo_space = GizmoSpace_Local;
        }
    }

    time_since_selected += frame_time;

    v2 mouse_viewport_position = get_mouse_viewport_position();
    i32 x_pos = (i32)mouse_viewport_position.x;
    i32 y_pos = (i32)mouse_viewport_position.y;

    editor_is_mouse_within_viewport = x_pos >= 0 && x_pos <= render_surface_width && y_pos >= 0 && y_pos <= render_surface_height;

    i32 index = gizmo_in_use_index;
    if (gizmo_in_use_index == NOTHING_SELECTED_INDEX && editor_is_mouse_within_viewport)
    {
        index = read_editor_index(x_pos, y_pos);
    }

    // @TODO(dubgron): Change it into something like input_get_state(Mouse_Button1).
    if (editor_is_mouse_within_viewport && input_is_pressed(Mouse_Button1))
    {
        if (index > NOTHING_SELECTED_INDEX)
        {
            if (selected_entity.index != index)
            {
                i32 generation = world.entity_list[index].generation;
                selected_entity = EntityID{ index, generation };

                time_since_selected = 0.f;
            }
        }
        else if (index == NOTHING_SELECTED_INDEX)
        {
            selected_entity = EntityID{};
        }
        else switch (index)
        {
            case GIZMO_X_AXIS_INDEX:
            case GIZMO_Y_AXIS_INDEX:
            case GIZMO_XY_AXIS_INDEX:
            {
                Entity* entity = entity_get(&world, selected_entity);
                APORIA_ASSERT(entity);

                gizmo_in_use_index = index;
                mouse_start_position = get_mouse_world_position();
                entity_start_position = entity->position;
                entity_start_rotation = entity->rotation;
                entity_start_scale = entity->scale;
            }
        }
    }
    else if (gizmo_in_use_index != NOTHING_SELECTED_INDEX && input_is_held(Mouse_Button1))
    {
        Entity* entity = entity_get(&world, selected_entity);
        APORIA_ASSERT(entity);

        v2 mouse_current_position = get_mouse_world_position();

        v2 mouse_start_offset = mouse_start_position - entity_start_position;
        v2 mouse_current_offset = mouse_current_position - entity_start_position;

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
                        entity->position = entity_start_position + glm::dot(mouse_offset, right) * right;
                    }
                    break;

                    case GIZMO_Y_AXIS_INDEX:
                    {
                        entity->position = entity_start_position + glm::dot(mouse_offset, up) * up;
                    }
                    break;

                    case GIZMO_XY_AXIS_INDEX:
                    {
                        entity->position = entity_start_position + mouse_offset;
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

                entity->rotation = entity_start_rotation + (current_angle - base_angle);
            }
            break;

            case GizmoType_Scale:
            {
                v2 scale = mouse_current_offset / mouse_start_offset;

                switch (index)
                {
                    case GIZMO_X_AXIS_INDEX:
                    {
                        entity->scale.x = entity_start_scale.x * scale.x;

                        if (input_is_held(Key_LShift))
                        {
                            entity->scale.y = entity_start_scale.y * scale.x;
                        }

                    }
                    break;

                    case GIZMO_Y_AXIS_INDEX:
                    {
                        entity->scale.y = entity_start_scale.y * scale.y;

                        if (input_is_held(Key_LShift))
                        {
                            entity->scale.x = entity_start_scale.x * scale.y;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    else if (input_is_released(Mouse_Button1))
    {
        gizmo_in_use_index = NOTHING_SELECTED_INDEX;
    }
}

void editor_draw_frame(f32 frame_time)
{
    if (!editor_is_open)
        return;

    ImGui::Begin("Tools");

    if (ImGui::RadioButton("World", gizmo_space == GizmoSpace_World))
    {
        gizmo_space = GizmoSpace_World;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", gizmo_space == GizmoSpace_Local))
    {
        gizmo_space = GizmoSpace_Local;
    }

    ImGui::Checkbox("Is editor open?", &editor_is_open);

    ImGui::End();

    if (selected_entity.index == NOTHING_SELECTED_INDEX)
    {
        return;
    }

    if (Entity* entity = entity_get(&world, selected_entity))
    {
        // @HACK(dubgron): It's stupid and there should be an easier way to do it.
        u32 shader = entity->shader_id;
        f32 z = entity->z;
        entity->shader_id = editor_selected_shader;
        entity->z = 0.99f;
        draw_entity(*entity);
        entity->shader_id = shader;
        entity->z = z;

        f32 camera_zoom = active_camera->projection.zoom;

        // Draw gizmos
        f32 line_thickness = 5.f * camera_zoom;
        f32 line_length = 100.f * camera_zoom;
        f32 end_size = 25.f * camera_zoom;

        v2 start = entity->position;

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

                // Draw XY axis
                {
                    f32 point_radius = 7.5f * camera_zoom;

                    set_editor_index(GIZMO_XY_AXIS_INDEX);
                    draw_circle(start, point_radius, Color::White);

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

                if (gizmo_in_use_index != NOTHING_SELECTED_INDEX)
                {
                    v2 mouse_start_offset = mouse_start_position - entity_start_position;
                    v2 mouse_current_offset = get_mouse_world_position() - entity_start_position;

                    f32 rotation_line_thickness = 3.f * camera_zoom;

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
                v2 right = v2{ cos(entity->rotation), sin(entity->rotation) };
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
}
