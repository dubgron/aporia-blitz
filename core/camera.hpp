#pragma once

#include <glm/glm.hpp>

namespace Aporia
{
    class Camera final
    {
    public:
        Camera(float left, float right, float bottom, float top);

        void set_projection_matrix(float left, float right, float bottom, float top);
        void set_view_matrix(const glm::vec2& position, float rotation);

        void set_frustum(float left, float right, float bottom, float top);

        void set_position(glm::vec2 pos);
        void move(const glm::vec2& vec);
        const glm::vec2& get_position() const;

        void set_rotation(float rotation);
        void rotate(float rotation);
        float get_rotation() const;

        void set_zoom(float zoom);
        void zoom(float zoom);
        float get_zoom() const;

        const glm::mat4& get_projection_matrix() const;
        const glm::mat4& get_view_matrix() const;
        const glm::mat4& get_view_projection_matrix() const;

    private:
        glm::mat4 _projection_matrix = glm::mat4{ 1.0f };
        glm::mat4 _view_matrix = glm::mat4{ 1.0f };
        glm::mat4 _view_projection_matrix = glm::mat4{ 1.0f };

        float _left, _right, _bottom, _top;
        glm::vec2 _position = glm::vec2{ 0.0f };
        float _rotation = 0.0f;
        float _zoom = 1.0f;
    };
}
