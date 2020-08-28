#pragma once

#include <cstddef>
#include <stack>
#include <type_traits>

#include <GL/gl3w.h>
#include <glm/matrix.hpp>

#include "camera.hpp"
#include "logger.hpp"
#include "components/circular.hpp"
#include "components/color.hpp"
#include "components/linear2d.hpp"
#include "components/rectangular.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"
#include "graphics/group.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex.hpp"
#include "graphics/vertex_array.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Renderer final
    {
        using Buffer = std::vector<Vertex>;
        using BufferIter = Buffer::iterator;
        using BufferRange = std::pair<BufferIter, BufferIter>;

        static constexpr std::size_t MAX_QUEUE = 10000;

    public:
        Renderer(Logger& logger);

        void begin(const Camera& camera);
        void end();
        void render();

        void draw(const Group& group);
        void draw(const Sprite& sprite);
        void draw(const Rectangle2D& rect);
        void draw(const Line2D& line);
        void draw(const Circle2D& circle);

        void push_transform(const Transform2D& transform);
        void pop_transform();

        Shader& get_default_shader() { return _default_shader; }

    private:
        Logger& _logger;

        VertexArray<MAX_QUEUE, 4, 6> _opaque_quads;
        VertexArray<MAX_QUEUE, 4, 6> _transpartent_quads;
        VertexArray<MAX_QUEUE, 2, 2> _lines;

        std::stack<glm::mat4> _tranformation_stack;

        Shader _default_shader;
    };
}
