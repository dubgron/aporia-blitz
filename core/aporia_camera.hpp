#pragma once

#include "aporia_types.hpp"

namespace Aporia
{
    struct CameraView
    {
        v2 position{ 0.f };
        f32 rotation = 0.f;

        // Cached values
        mutable m4 matrix{ 1.f };

        mutable v2 up_vector{ 0.f, 1.f };
        mutable v2 right_vector{ 1.f, 0.f };
    };

    struct CameraProjection
    {
        f32 fov = 1.f;
        f32 aspect_ratio = 1.f;
        f32 zoom = 1.f;

        // Cached values
        mutable m4 matrix{ 1.f };
    };

    enum CameraDirtyFlag : u8
    {
        CameraDirtyFlag_View        = 1 << 0,
        CameraDirtyFlag_Projection  = 1 << 1
    };

    struct Camera
    {
        CameraView view;
        CameraProjection projection;

        m4 view_projection_matrix{ 1.f };
        u8 dirty_flags = CameraDirtyFlag_View | CameraDirtyFlag_Projection;

        void mark_as_dirty(CameraDirtyFlag flag) { dirty_flags |= flag; }
        bool is_marked_dirty(CameraDirtyFlag flag) { return dirty_flags & flag; }

        void init();

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

        void on_window_resize(u32 width, u32 height);
    };
}
