#pragma once

#include <functional>
#include <stack>
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
        static constexpr size_t MAX_QUEUE = 10000;

    public:
        Renderer(Logger& logger, ShaderManager& shaders);

        void begin(const Camera& camera);
        void end();

        void flush(Shader program_id, BufferType buffer);
        void render();

        void draw(const Group& group);
        void draw(const Sprite& sprite, Shader program_id = 0);
        void draw(const Rectangle2D& rect, Shader program_id = 0);
        void draw(const Line2D& line, Shader program_id = 0);
        void draw(const Circle2D& circle, Shader program_id = 0);
        void draw(const Text& text, Shader program_id = 0);

        void push_transform(const Transform2D& transform);
        void pop_transform();

    private:
        Logger& _logger;
        ShaderManager& _shaders;

        RenderQueue _render_queue;

        VertexArray<MAX_QUEUE, 4, 6> _quads;
        VertexArray<MAX_QUEUE, 2, 2> _lines;

        std::stack<glm::mat4> _tranformation_stack;
    };
}
