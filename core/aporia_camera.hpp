#pragma once

#include "aporia_types.hpp"
#include "aporia_memory.hpp"

struct CameraView
{
    v2 position{ 0.f };
    f32 rotation = 0.f;

    // Cached values
    m4 matrix{ 1.f };

    v2 up_vector{ 0.f, 1.f };
    v2 right_vector{ 1.f, 0.f };
};

struct CameraProjection
{
    f32 fov = 1.f;
    f32 aspect_ratio = 1.f;
    f32 zoom = 1.f;

    // Cached values
    m4 matrix{ 1.f };
};

using CameraDirtyFlags = u8;
enum CameraDirtyFlag_ : CameraDirtyFlags
{
    CameraDirtyFlag_View        = 0x01,
    CameraDirtyFlag_Projection  = 0x02,
};

struct Camera
{
    CameraView view;
    CameraProjection projection;

    m4 view_projection_matrix{ 1.f };
    CameraDirtyFlags dirty_flags = CameraDirtyFlag_View | CameraDirtyFlag_Projection;

    void mark_as_dirty(CameraDirtyFlags flag) { dirty_flags |= flag; }
    bool is_marked_dirty(CameraDirtyFlags flag) { return dirty_flags & flag; }

    const m4& calculate_view_projection_matrix();

    void set_position(v2 new_position);
    void move(v2 vector);

    void set_rotation(f32 new_rotation);
    void rotate(f32 rotation);

    void set_fov(f32 new_fov);
    void set_aspect_ratio(f32 new_aspect_ratio);
    void set_zoom(f32 new_zoom);
    void zoom(f32 zoom);

    void control_movement(f32 delta_time);
    void control_rotation(f32 delta_time);
    void control_zoom(f32 delta_time);
    void follow(v2 to_follow, f32 delta_time);

    void apply_config();
    void adjust_aspect_ratio_to_render_surface();
};

Camera* create_camera(MemoryArena* arena);

extern Camera* active_camera;
