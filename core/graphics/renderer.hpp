#pragma once

#include <functional>
#include <utility>
#include <vector>

#include <glm/matrix.hpp>

#include "camera.hpp"
#include "logger.hpp"
#include "shader_manager.hpp"
#include "components/transform2d.hpp"
#include "graphics/group.hpp"
#include "graphics/render_queue.hpp"
#include "graphics/text.hpp"
#include "graphics/vertex.hpp"
#include "graphics/vertex_array.hpp"

namespace Aporia
{
    class Renderer final
    {
        static constexpr size_t MAX_QUEUE = 100000u;

    public:
        Renderer(Logger& logger, ShaderManager& shaders);

        void begin(Camera& camera);
        void end();

        void flush(Shader program_id, BufferType buffer);

        void draw(const Group& group);
        void draw(const Sprite& sprite,     Shader program_id = default_shader);
        void draw(const Rectangle2D& rect,  Shader program_id = default_shader);
        void draw(const Line2D& line,       Shader program_id = default_shader);
        void draw(const Circle2D& circle,   Shader program_id = default_shader);
        void draw(const Text& text,         Shader program_id = font_shader);

        void push_transform(const Transform2D& transform);
        void pop_transform();

    private:
        Logger& _logger;
        ShaderManager& _shaders;

        RenderQueue _render_queue;

        std::vector<VertexArray> _vertex_arrays;

        std::vector<Transform2D> _transformation_stack;

        /* Predefined shaders */
        static ShaderRef default_shader;
        static ShaderRef font_shader;
    };
}
