#pragma once

#include "aporia_types.hpp"

namespace Aporia
{
    struct CameraView final
    {
        v2 position{ 0.f };
        f32 rotation = 0.f;

        /* Cached Values */
        mutable m4 matrix{ 1.f };

        mutable v2 up_vector{ 0.f, 1.f };
        mutable v2 right_vector{ 1.f, 0.f };
    };

    struct CameraProjection final
    {
        f32 fov = 1.f;
        f32 aspect_ratio = 1.f;
        f32 zoom = 1.f;

        /* Cached Values */
        mutable m4 matrix{ 1.f };
    };

    class Camera final
    {
    public:
        void init(f32 fov, f32 aspect_ratio);

        const m4& get_view_projection_matrix() const;

        void set_position(v2 new_position);
        void move(const v2& vector);
        const v2& get_position() const;

        void set_rotation(f32 new_rotation);
        void rotate(f32 rotation);
        f32 get_rotation() const;

        const v2& get_up_vector() const;
        const v2& get_right_vector() const;

        void set_fov(f32 new_fov);
        f32 get_fov() const;

        void set_aspect_ratio(f32 new_aspect_ratio);
        f32 get_aspect_ratio() const;

        void set_zoom(f32 new_zoom);
        void zoom(f32 zoom);
        f32 get_zoom() const;

    private:
        void recalculate_view() const;
        void recalculate_projection() const;

    private:
        CameraView _view;
        CameraProjection _projection;

        mutable m4 _vp_matrix{ 1.f };

    private:
        /* Dirty Flags for Matrices */
        using DirtyFlag = uint8_t;

        static constexpr DirtyFlag DIRTYFLAG_VIEW       = 1 << 0;
        static constexpr DirtyFlag DIRTYFLAG_PROJECTION = 1 << 1;

    private:
        void mark_as_dirty(DirtyFlag flag) const     { _dirty_flags |= flag; }
        bool is_marked_dirty(DirtyFlag flag) const   { return _dirty_flags & flag; }

    private:
        mutable DirtyFlag _dirty_flags = DIRTYFLAG_VIEW | DIRTYFLAG_PROJECTION;
    };
}
