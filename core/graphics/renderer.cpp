#include "renderer.hpp"

#include <array>
#include <cmath>
#include <numeric>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "utils/math.hpp"

namespace Aporia
{
    Renderer::Renderer(Logger& logger)
        : _logger(logger), _default_shader(_logger)
    {
        _tranformation_stack.emplace(glm::mat4{ 1.0f });

        /* Set VertexArray for opaque Quads */
        _opaque_quads.bind();

        auto opaque_quads_vbo = std::make_shared<VertexBuffer<MAX_QUEUE, 4>>();
        opaque_quads_vbo->add_layout();
        _opaque_quads.set_vertex_buffer(opaque_quads_vbo);

        std::vector<uint32_t> quad_indecies(MAX_QUEUE * 6);
        for (size_t i = 0, offset = 0; i < MAX_QUEUE * 6; i += 6, offset += 4)
        {
            quad_indecies[  i  ] = offset + 0;
            quad_indecies[i + 1] = offset + 1;
            quad_indecies[i + 2] = offset + 2;

            quad_indecies[i + 3] = offset + 2;
            quad_indecies[i + 4] = offset + 3;
            quad_indecies[i + 5] = offset + 0;
        }

        auto opaque_quads_ibo = std::make_shared<IndexBuffer<MAX_QUEUE, 6>>(quad_indecies);
        _opaque_quads.set_index_buffer(opaque_quads_ibo);

        _opaque_quads.unbind();

        /* Set VertexArray for transparent Quads */
        _transpartent_quads.bind();

        auto transpartent_quads_vbo = std::make_shared<VertexBuffer<MAX_QUEUE, 4>>();
        transpartent_quads_vbo->add_layout();
        _transpartent_quads.set_vertex_buffer(transpartent_quads_vbo);

        auto transpartent_quads_ibo = std::make_shared<IndexBuffer<MAX_QUEUE, 6>>(quad_indecies);
        _transpartent_quads.set_index_buffer(transpartent_quads_ibo);

        _transpartent_quads.unbind();

        /* Set VertexArray for Lines */
        _lines.bind();

        auto lines_vbo = std::make_shared<VertexBuffer<MAX_QUEUE, 2>>();
        lines_vbo->add_layout();
        _lines.set_vertex_buffer(lines_vbo);

        std::vector<uint32_t> line_indecies(MAX_QUEUE * 2);
        for (size_t i = 0; i < MAX_QUEUE * 2; ++i)
            line_indecies[i] = i;

        auto lines_ibo = std::make_shared<IndexBuffer<MAX_QUEUE, 2>>(line_indecies);
        _lines.set_index_buffer(lines_ibo);

        _lines.unbind();

        /* Setup default shaders */
        _default_shader.load_shader("shaders/shader.frag", Aporia::Shader::Type::Fragment);
        _default_shader.load_shader("shaders/shader.vert", Aporia::Shader::Type::Vertex);
        _default_shader.compile();

        std::array<int32_t, OPENGL_MAX_TEXTURE_UNITS> sampler;
        std::iota(sampler.begin(), sampler.end(), 0);

        _default_shader.bind();
        _default_shader.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);
        _default_shader.unbind();
    }

    void Renderer::begin(const Camera& camera)
    {
        _default_shader.bind();
        _default_shader.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
    }

    void Renderer::end()
    {
        auto transparent_buffer = _transpartent_quads.get_vertex_buffer();
        std::stable_sort(transparent_buffer->begin(), transparent_buffer->end(), [](const Vertex& v_1, const Vertex& v_2){ return v_1.position.z < v_2.position.z; });

        render();
    }

    void Renderer::render()
    {
        _opaque_quads.render();
        _lines.render();

        _transpartent_quads.render();
    }

    void Renderer::draw(const Group& group)
    {
        push_transform(group.get_component<Transform2D>());

        for (const Line2D& line : group.get_container<Line2D>())
            draw(line);

        for (const Rectangle2D& rectangle : group.get_container<Rectangle2D>())
            draw(rectangle);

        for (const Sprite& sprite : group.get_container<Sprite>())
            draw(sprite);

        for (const Group& group : group.get_container<Group>())
            draw(group);

        pop_transform();
    }

    void Renderer::draw(const Sprite& sprite)
    {
        const Transform2D& transform = sprite.get_component<Transform2D>();
        const Rectangular& rectangular = sprite.get_component<Rectangular>();
        const Texture& texture = sprite.get_component<Texture>();
        const Color& color = sprite.get_component<Color>();

        auto& quads = color.a == 255 && texture.origin.channels % 2 ? _opaque_quads : _transpartent_quads;

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = texture.u;
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ rectangular.width, 0.0f, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = glm::vec2{ texture.v.x, texture.u.y };
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ rectangular.width, rectangular.height, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = texture.v;
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ 0.0f, rectangular.height, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = glm::vec2{ texture.u.x, texture.v.y };
        v.color = color;
        quads.get_vertex_buffer()->push(v);
    }

    void Renderer::draw(const Rectangle2D& rect)
    {
        const Transform2D& transform = rect.get_component<Transform2D>();
        const Rectangular& rectangular = rect.get_component<Rectangular>();
        const Color& color = rect.get_component<Color>();

        auto& quads = color.a == 255 ? _opaque_quads : _transpartent_quads;

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ rectangular.width, 0.0f, 0.0f, 1.0f };
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ rectangular.width, rectangular.height, 0.0f, 1.0f };
        v.color = color;
        quads.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ 0.0f, rectangular.height, 0.0f, 1.0f };
        v.color = color;
        quads.get_vertex_buffer()->push(v);
    }

    void Renderer::draw(const Line2D& line2d)
    {
        const Transform2D& transform = line2d.get_component<Transform2D>();
        const Linear2D& linear2d = line2d.get_component<Linear2D>();
        const Color& color = line2d.get_component<Color>();

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.color = color;
        _lines.get_vertex_buffer()->push(v);

        v.position = transformation * glm::vec4{ linear2d.point, 0.0f, 1.0f };
        v.color = color;
        _lines.get_vertex_buffer()->push(v);
    }

    void Renderer::draw(const Circle2D& circle)
    {
        const Transform2D& transform = circle.get_component<Transform2D>();
        const Circular& circular = circle.get_component<Circular>();
        const Color& color = circle.get_component<Color>();

        auto& circles = color.a == 255 ? _opaque_quads : _transpartent_quads;

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        Vertex v;
        v.color = color;
        v.tex_coord = glm::vec2{ -1.0f, -1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        circles.get_vertex_buffer()->push(v);

        v.color = color;
        v.tex_coord = glm::vec2{ 1.0f, -1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        circles.get_vertex_buffer()->push(v);

        v.color = color;
        v.tex_coord = glm::vec2{ 1.0f, 1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        circles.get_vertex_buffer()->push(v);

        v.color = color;
        v.tex_coord = glm::vec2{ -1.0f, 1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        circles.get_vertex_buffer()->push(v);
    }

    void Renderer::push_transform(const Transform2D& transform)
    {
        _tranformation_stack.push(std::move(_tranformation_stack.top() * to_mat4(transform)));
    }

    void Renderer::pop_transform()
    {
        if (_tranformation_stack.size() > 1)
            _tranformation_stack.pop();
    }
}
