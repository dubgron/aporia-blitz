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
};

const m4& camera_calculate_view_projection_matrix(Camera* camera);

void camera_control_movement(Camera* camera);
void camera_control_rotation(Camera* camera);
void camera_control_zoom(Camera* camera, f32 delta_time);
void camera_follow(Camera* camera, v2 to_follow, f32 delta_time);

void camera_apply_config(Camera* camera);
void camera_adjust_aspect_ratio_to_render_surface(Camera* camera);

extern Camera active_camera;
