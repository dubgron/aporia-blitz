#include "aporia_editor.hpp"

#include "imgui_internal.h"

#include "aporia_camera.hpp"
#include "aporia_game.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"
#include "aporia_world.hpp"

bool editor_is_open = true;
f32 editor_grid_size = 100.f;

EntityID selected_entity_id;
f32 time_since_selected = 0.f;

enum GizmoType : u8
{
    Gizmo_Translate,
    Gizmo_Rotate,
    Gizmo_Scale,
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
static GizmoType displayed_gizmo_type = Gizmo_Translate;
static GizmoSpace gizmo_space = GizmoSpace_World;

static v2 initial_mouse_position{ 0.f };
static Entity initial_entity_state;

enum EditorActionType : u32
{
    EditorAction_Invalid,
    EditorAction_SelectEntity,
    EditorAction_ModifyEntity,
};

struct EditorAction
{
    EditorActionType type = EditorAction_Invalid;

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

    selected_entity_id = new_entity_id;

    EditorAction* action = editor_make_new_action();
    action->type = EditorAction_SelectEntity;
    action->entity_state.id = selected_entity_id;

    if (selected_entity_id.index != INDEX_INVALID)
    {
        action->entity_state = *entity_get(&current_world, selected_entity_id);
    }
}

static void editor_modify_entity(Entity* entity)
{
    EditorAction* action = editor_make_new_action();
    action->type = EditorAction_ModifyEntity;
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
        case EditorAction_SelectEntity:
        {
            selected_entity_id = prev_entity_state->id;
        }
        break;

        case EditorAction_ModifyEntity:
        {
            Entity* entity = entity_get(&current_world, selected_entity_id);
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
        case EditorAction_SelectEntity:
        {
            selected_entity_id = last_action.entity_state.id;
        }
        break;

        case EditorAction_ModifyEntity:
        {
            Entity* entity = entity_get(&current_world, selected_entity_id);
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

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(viewport);

    static const ImGuiID viewport_id = ImHashStr("Viewport");

    ImGuiContext* context = ImGui::GetCurrentContext();
    bool mouse_within_viewport = context->HoveredWindow && (context->HoveredWindow->ID == viewport_id);
    //bool focused_on_viewport = context->NavWindow && (context->NavWindow->ID == viewport_id);

    if (mouse_within_viewport)
    {
        active_camera->control_movement(frame_time);
        active_camera->control_rotation(frame_time);
        active_camera->control_zoom(frame_time);
    }

    i32 index = gizmo_index;
    if (gizmo_index == NOTHING_SELECTED_INDEX && mouse_within_viewport)
    {
        index = read_editor_index();
    }

    InputState left_mouse_button = input_get(Mouse_Button1);

    if (!input_is_held(left_mouse_button))
    {
        if (input_is_pressed(Key_Num1))
        {
            displayed_gizmo_type = Gizmo_Translate;
        }
        else if (input_is_pressed(Key_Num2))
        {
            displayed_gizmo_type = Gizmo_Rotate;
        }
        else if (input_is_pressed(Key_Num3))
        {
            displayed_gizmo_type = Gizmo_Scale;
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

    v2 mouse_current_position = get_mouse_world_position();

    if (mouse_within_viewport && input_is_pressed(left_mouse_button))
    {
        if (index > NOTHING_SELECTED_INDEX)
        {
            i32 generation = current_world.entity_array[index].id.generation;
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
            Entity* entity = entity_get(&current_world, selected_entity_id);
            APORIA_ASSERT(entity);

            gizmo_index = index;
            initial_mouse_position = get_mouse_world_position();
            initial_entity_state = *entity;
        }
    }
    else if (gizmo_index != NOTHING_SELECTED_INDEX)
    {
        Entity* entity = entity_get(&current_world, selected_entity_id);
        APORIA_ASSERT(entity);

        if (input_is_held(left_mouse_button))
        {
            v2 mouse_start_offset = initial_mouse_position - initial_entity_state.position;
            v2 mouse_current_offset = mouse_current_position - initial_entity_state.position;

            switch (gizmo_index)
            {
                case TRANSLATE_X_AXIS_INDEX:
                case TRANSLATE_Y_AXIS_INDEX:
                case TRANSLATE_XY_AXIS_INDEX:
                {
                    v2 right = v2{ 1.f, 0.f };
                    v2 up = v2{ 0.f, 1.f };

                    if (gizmo_space == GizmoSpace_Local)
                    {
                        right = v2{ cos(entity->rotation), sin(entity->rotation) };
                        up = v2{ -right.y, right.x };
                    }

                    v2 mouse_offset = mouse_current_position - initial_mouse_position;

                    bool snap_to_grid = input_is_held(Key_LShift);
                    f32 snap_resolution = editor_grid_size / 2.f;

                    entity->position = initial_entity_state.position;

                    if (gizmo_index == TRANSLATE_X_AXIS_INDEX || gizmo_index == TRANSLATE_XY_AXIS_INDEX)
                    {
                        entity->position += glm::dot(mouse_offset, right) * right;

                        if (snap_to_grid)
                            entity->position.x = std::floor(mouse_current_position.x / snap_resolution) * snap_resolution;
                    }

                    if (gizmo_index == TRANSLATE_Y_AXIS_INDEX || gizmo_index == TRANSLATE_XY_AXIS_INDEX)
                    {
                        entity->position += glm::dot(mouse_offset, up) * up;

                        if (snap_to_grid)
                            entity->position.y = std::floor(mouse_current_position.y / snap_resolution) * snap_resolution;
                    }
                }
                break;

                case ROTATE_INDEX:
                {
                    f32 base_angle = atan2(mouse_start_offset.y, mouse_start_offset.x);
                    f32 current_angle = atan2(mouse_current_offset.y, mouse_current_offset.x);

                    entity->rotation = initial_entity_state.rotation + (current_angle - base_angle);
                }
                break;

                case SCALE_X_AXIS_INDEX:
                case SCALE_Y_AXIS_INDEX:
                {
                    v2 scale{ 1.f };

                    v2 right = v2{ cos(entity->rotation), sin(entity->rotation) };
                    v2 up = v2{ -right.y, right.x };

                    switch (gizmo_index)
                    {
                        case SCALE_X_AXIS_INDEX:
                        {
                            scale.x = glm::dot(mouse_current_offset, right) / glm::dot(mouse_start_offset, right);

                            if (input_is_held(Key_LShift))
                                scale.y = scale.x;
                        }
                        break;

                        case SCALE_Y_AXIS_INDEX:
                        {
                            scale.y = glm::dot(mouse_current_offset, up) / glm::dot(mouse_start_offset, up);

                            if (input_is_held(Key_LShift))
                                scale.x = scale.y;
                        }
                        break;
                    }

                    entity->scale = initial_entity_state.scale * scale;
                }
                break;
            }
        }
        else if (input_is_released(left_mouse_button))
        {
            editor_modify_entity(entity);
            gizmo_index = NOTHING_SELECTED_INDEX;
        }
    }

    ImGui::Begin("Tools");
    {
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

        ImGui::Separator();

        if (ImGui::RadioButton("Translate", displayed_gizmo_type == Gizmo_Translate))
        {
            displayed_gizmo_type = Gizmo_Translate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", displayed_gizmo_type == Gizmo_Rotate))
        {
            displayed_gizmo_type = Gizmo_Rotate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", displayed_gizmo_type == Gizmo_Scale))
        {
            displayed_gizmo_type = Gizmo_Scale;
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
    }
    ImGui::End();

    ImGui::Begin("World");
    {
        float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("##scrolling_region", ImVec2(0, -footer_height_to_reserve), true))
        {
            for (u64 idx = 0; idx < current_world.entity_count; ++idx)
            {
                const Entity& entity = current_world.entity_array[idx];
                if (!entity_flags_has_all(entity, EntityFlag_Active))
                    continue;

                String name = tprintf("Entity (ID = %, GEN = %)", entity.id.index, entity.id.generation);
                if (ImGui::Selectable(*name, selected_entity_id == entity.id))
                    selected_entity_id = entity.id;
            }

            ImGui::EndChild();
        }

        if (ImGui::Button("Create Entity"))
        {
            selected_entity_id = entity_create(&current_world);
        }
        if (selected_entity_id.index != INDEX_INVALID)
        {
            ImGui::SameLine();
            if (ImGui::Button("Destroy Entity"))
            {
                entity_destroy(&current_world, selected_entity_id);
                selected_entity_id = EntityID{};
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy Entity"))
            {
                Entity* entity = entity_get(&current_world, selected_entity_id);
                APORIA_ASSERT(entity);

                Entity* new_entity = nullptr;
                selected_entity_id = entity_create(&current_world, &new_entity);
                EntityID id = new_entity->id;
                *new_entity = *entity;
                new_entity->id = id;
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Properties");
    {
        if (selected_entity_id.index != INDEX_INVALID)
        {
            Entity* selected_entity = entity_get(&current_world, selected_entity_id);
            u32 before_hash = get_hash(selected_entity, sizeof(Entity));

            if (ImGui::BeginCombo("Entity Type", *entity_type_to_string(selected_entity->type)))
            {
                for (u32 n = 0; n < EntityType_Count; ++n)
                {
                    EntityType type = (EntityType)n;
                    bool is_selected = (selected_entity->type == type);

                    if (ImGui::Selectable(*entity_type_to_string(type), is_selected))
                        selected_entity->type = type;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (selected_entity->type != Entity_None)
            {
                ImGui::DragFloat3("Position", &selected_entity->position[0]);

                ImGui::DragFloat("Rotation", &selected_entity->rotation);
                ImGui::DragFloat2("Center of Rotation", &selected_entity->center_of_rotation[0]);

                ImGui::DragFloat2("Size", &selected_entity->width);
                ImGui::DragFloat2("Scale", &selected_entity->scale[0]);

                ImGui::Separator();

                static bool fit_size_by_default = false;

                if (ImGui::BeginCombo("Texture", *get_subtexture_name(selected_entity->texture)))
                {
                    struct SortedTexture
                    {
                        SubTexture texture;
                        String name = nullptr;
                    };

                    SortedTexture* textures = arena_push_uninitialized<SortedTexture>(&memory.frame, subtextures.valid_buckets);
                    i64 textures_count = 0;

                    for (i64 idx = 0; idx < subtextures.bucket_count; ++idx)
                    {
                        const SubTexture& texture = subtextures.buckets[idx].value;
                        if (texture.texture_index == INDEX_INVALID)
                            continue;

                        textures[textures_count].texture = texture;
                        textures[textures_count].name = get_subtexture_name(texture);
                        textures_count += 1;
                    }

                    qsort(textures, textures_count, sizeof(SortedTexture),
                        [](const void* elem0, const void* elem1) -> i32
                        {
                            SortedTexture* texture0 = (SortedTexture*)elem0;
                            SortedTexture* texture1 = (SortedTexture*)elem1;
                            return strcmp(*texture0->name, *texture1->name);
                        });

                    for (i64 idx = 0; idx < textures_count; ++idx)
                    {
                        const SubTexture& texture = textures[idx].texture;
                        bool is_selected = (texture == selected_entity->texture);

                        String name = textures[idx].name;
                        if (ImGui::Selectable(*name, is_selected))
                        {
                            selected_entity->texture = texture;

                            if (fit_size_by_default)
                                entity_adjust_size_to_texture(selected_entity);
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Fit Size to Texture"))
                    entity_adjust_size_to_texture(selected_entity);

                ImGui::SameLine();
                ImGui::Checkbox("Fit by Default", &fit_size_by_default);

                ImGui::Separator();

                v4 color = vec4_from_color(selected_entity->color);
                ImGui::ColorEdit4("Color", &color[0]);
                selected_entity->color = color_from_vec4(color);
            }

            // @TODO(dubgron): Action should be registered only when editing is finished.
            u32 after_hash = get_hash(selected_entity, sizeof(Entity));
            if (before_hash != after_hash)
            {
                editor_modify_entity(selected_entity);
            }
        }
    }
    ImGui::End();
}

void editor_draw_selected_entity()
{
    Entity* entity = entity_get(&current_world, selected_entity_id);
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
    Entity* entity = entity_get(&current_world, selected_entity_id);
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
        case Gizmo_Translate:
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

        case Gizmo_Rotate:
        {
            f32 radius = line_length;
            f32 inner_radius = radius - line_thickness;

            set_editor_index(ROTATE_INDEX);
            draw_circle(start, radius, inner_radius, GIZMO_XY_AXIS_COLOR);

            if (gizmo_index == ROTATE_INDEX)
            {
                v2 mouse_start_offset = initial_mouse_position - initial_entity_state.position;
                v2 mouse_current_offset = get_mouse_world_position() - initial_entity_state.position;

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

        case Gizmo_Scale:
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
