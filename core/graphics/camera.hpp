#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace Aporia
{
    struct CameraView final
    {
        glm::vec2 position{ 0.0f };
        float rotation = 0.0f;

        /* Cached Values */
        mutable glm::mat4 matrix{ 1.0f };

        mutable glm::vec2 up_vector{ 0.0f, 1.0f };
        mutable glm::vec2 right_vector{ 1.0f, 0.0f };
    };

    struct CameraProjection final
    {
        float fov = 1.0f;
        float aspect_ratio = 1.0f;
        float zoom = 1.0f;

        /* Cached Values */
        mutable glm::mat4 matrix{ 1.0f };
    };

    class Camera final
    {
    public:
        Camera(float fov, float aspect_ratio);

        const glm::mat4& get_view_projection_matrix() const;

        void set_position(glm::vec2 new_position);
        void move(const glm::vec2& vector);
        const glm::vec2& get_position() const;

        void set_rotation(float new_rotation);
        void rotate(float rotation);
        float get_rotation() const;

        const glm::vec2& get_up_vector() const;
        const glm::vec2& get_right_vector() const;

        void set_fov(float new_fov);
        float get_fov() const;

        void set_aspect_ratio(float new_aspect_ratio);
        float get_aspect_ratio() const;

        void set_zoom(float new_zoom);
        void zoom(float zoom);
        float get_zoom() const;

    private:
        void recalculate_view() const;
        void recalculate_projection() const;

        CameraView _view;
        CameraProjection _projection;

        mutable glm::mat4 _vp_matrix{ 1.0f };

        /* Dirty Flags for Matrices */
        using DirtyFlag = uint8_t;

        void mark_as_dirty(DirtyFlag flag) const     { _dirty_flags |= flag; }
        bool is_marked_dirty(DirtyFlag flag) const   { return _dirty_flags & flag; }

        static constexpr DirtyFlag DIRTYFLAG_VIEW       = 1 << 0;
        static constexpr DirtyFlag DIRTYFLAG_PROJECTION = 1 << 1;

        mutable DirtyFlag _dirty_flags = DIRTYFLAG_VIEW | DIRTYFLAG_PROJECTION;
    };
}
