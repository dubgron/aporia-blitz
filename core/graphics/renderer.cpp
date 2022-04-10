#include "renderer.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "utils/math.hpp"

namespace Aporia
{
    Renderer::Renderer(Logger& logger, ShaderManager& shaders)
        : _logger(logger), _shaders(shaders)
    {
        _tranformation_stack.emplace(glm::mat4{ 1.0f });

        /* Set VertexArray for opaque Quads */
        _quads.bind();

        auto quads_vbo = std::make_shared<VertexBuffer<MAX_QUEUE, 4>>();
        quads_vbo->add_layout();
        _quads.set_vertex_buffer(quads_vbo);

        std::vector<uint32_t> quad_indecies(MAX_QUEUE * 6);
        for (uint32_t i = 0, offset = 0; i < MAX_QUEUE * 6; i += 6, offset += 4)
        {
            quad_indecies[  i  ] = offset + 0;
            quad_indecies[i + 1] = offset + 1;
            quad_indecies[i + 2] = offset + 2;

            quad_indecies[i + 3] = offset + 2;
            quad_indecies[i + 4] = offset + 3;
            quad_indecies[i + 5] = offset + 0;
        }

        auto quads_ibo = std::make_shared<IndexBuffer<MAX_QUEUE, 6>>(quad_indecies);
        _quads.set_index_buffer(quads_ibo);

        _quads.unbind();

        /* Set VertexArray for Lines */
        _lines.bind();

        auto lines_vbo = std::make_shared<VertexBuffer<MAX_QUEUE, 2>>();
        lines_vbo->add_layout();
        _lines.set_vertex_buffer(lines_vbo);

        std::vector<uint32_t> line_indecies(MAX_QUEUE * 2);
        for (uint32_t i = 0; i < MAX_QUEUE * 2; ++i)
            line_indecies[i] = i;

        auto lines_ibo = std::make_shared<IndexBuffer<MAX_QUEUE, 2>>(line_indecies);
        _lines.set_index_buffer(lines_ibo);

        _lines.unbind();

        /* Setup default shaders */
        Shader default_shader = _shaders.create_program("default", "assets/shaders/default.shader");

        std::array<int32_t, OPENGL_MAX_TEXTURE_UNITS> sampler{};
        std::iota(sampler.begin(), sampler.end(), 0);

        _shaders.bind(default_shader);
        _shaders.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);

        /* Setup font shaders */
        Shader font_shader = _shaders.create_program("font", "assets/shaders/font.shader");

        _shaders.bind(font_shader);
        _shaders.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);

        _shaders.unbind();
    }

    void Renderer::begin(const Camera& camera)
    {
        _shaders.bind("default");
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());

        _shaders.bind("font");
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        _shaders.set_float("u_camera_zoom", 1.0f / camera.get_zoom());
    }

    void Renderer::end()
    {
        render();
    }

    void Renderer::flush(Shader program_id, BufferType buffer)
    {
        _shaders.bind(program_id);

        switch (buffer)
        {
            case BufferType::Quads:     _quads.render();    break;
            case BufferType::Lines:     _lines.render();    break;
        }
    }

    void Renderer::render()
    {
        RenderQueueKey prev_key;

        for (auto& [key, vertex] : _render_queue)
        {
            if (key.program_id != prev_key.program_id || key.buffer != prev_key.buffer)
            {
                flush(prev_key.program_id, prev_key.buffer);
            }

            if (key.buffer == BufferType::Quads)
            {
                _quads.get_vertex_buffer()->emplace(std::move(vertex));
            }
            else if (key.buffer == BufferType::Lines)
            {
                _lines.get_vertex_buffer()->emplace(std::move(vertex));
            }

            prev_key = key;
        }

        flush(prev_key.program_id, prev_key.buffer);

        _shaders.unbind();
        _render_queue.clear();
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

        for (const Text& text : group.get_container<Text>())
            draw(text);

        for (const Group& group : group.get_container<Group>())
            draw(group);

        pop_transform();
    }

    void Renderer::draw(const Sprite& sprite, Shader program_id /* = 0 */)
    {
        const Transform2D& transform = sprite.get_component<Transform2D>();
        const Rectangular& rectangular = sprite.get_component<Rectangular>();
        const Texture& texture = sprite.get_component<Texture>();
        const Color& color = sprite.get_component<Color>();

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        RenderQueueKey key;
        key.buffer = BufferType::Quads;
        key.program_id = program_id ? program_id : _shaders.get("default");
        key.transparent = color.a != 255 || texture.origin.channels % 2 == 0;

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = glm::vec2{ texture.u.x, texture.v.y };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ rectangular.width, 0.0f, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = texture.v;
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ rectangular.width, rectangular.height, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = glm::vec2{ texture.v.x, texture.u.y };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ 0.0f, rectangular.height, 0.0f, 1.0f };
        v.tex_id = texture.origin.id;
        v.tex_coord = texture.u;
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);
    }

    void Renderer::draw(const Rectangle2D& rect, Shader program_id /* = 0 */)
    {
        const Transform2D& transform = rect.get_component<Transform2D>();
        const Rectangular& rectangular = rect.get_component<Rectangular>();
        const Color& color = rect.get_component<Color>();

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        RenderQueueKey key;
        key.buffer = BufferType::Quads;
        key.program_id = program_id ? program_id : _shaders.get("default");
        key.transparent = color.a != 255;

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ rectangular.width, 0.0f, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ rectangular.width, rectangular.height, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ 0.0f, rectangular.height, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);
    }

    void Renderer::draw(const Line2D& line2d, Shader program_id /* = 0 */)
    {
        const Transform2D& transform = line2d.get_component<Transform2D>();
        const Linear2D& linear2d = line2d.get_component<Linear2D>();
        const Color& color = line2d.get_component<Color>();

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        RenderQueueKey key;
        key.buffer = BufferType::Lines;
        key.program_id = program_id ? program_id : _shaders.get("default");
        key.transparent = color.a != 255;

        Vertex v;
        v.position = transformation * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.position = transformation * glm::vec4{ linear2d.point, 0.0f, 1.0f };
        v.color = color;
        key.depth = v.position.z;
        _render_queue.emplace(key, v);
    }

    void Renderer::draw(const Circle2D& circle, Shader program_id /* = 0 */)
    {
        const Transform2D& transform = circle.get_component<Transform2D>();
        const Circular& circular = circle.get_component<Circular>();
        const Color& color = circle.get_component<Color>();

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        RenderQueueKey key;
        key.buffer = BufferType::Quads;
        key.program_id = program_id ? program_id : _shaders.get("default");
        key.transparent = true;

        Vertex v;
        v.color = color;
        v.tex_coord = glm::vec2{ -1.0f, -1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.color = color;
        v.tex_coord = glm::vec2{ 1.0f, -1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.color = color;
        v.tex_coord = glm::vec2{ 1.0f, 1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        key.depth = v.position.z;
        _render_queue.emplace(key, v);

        v.color = color;
        v.tex_coord = glm::vec2{ -1.0f, 1.0f };
        v.position = transformation * glm::vec4{ circular.radius * v.tex_coord, 0.0f, 1.0f };
        key.depth = v.position.z;
        _render_queue.emplace(key, v);
    }

    void Renderer::draw(const Text& text, Shader program_id /* = 0 */)
    {
        const Transform2D& transform = text.get_component<Transform2D>();
        const Color& color = text.get_component<Color>();

        const Font& font = text.font;
        const Font::Atlas& atlas = font.atlas;

        glm::mat4 transformation = _tranformation_stack.top() != glm::mat4{ 1.0f } ? _tranformation_stack.top() * to_mat4(transform) : to_mat4(transform);

        /* Adjust text scaling by the predefined atlas font size */
        transformation[0][0] /= atlas.font_size;
        transformation[1][1] /= atlas.font_size;

        RenderQueueKey key;
        key.buffer = BufferType::Quads;
        key.program_id = program_id ? program_id : _shaders.get("font");
        key.transparent = true;

        const glm::vec2 uv_length = glm::vec2{ atlas.origin.width, atlas.origin.height };

        const float text_scale = glm::length(transform.scale);
        const float screen_px_range = text_scale * font.atlas.distance_range / font.atlas.font_size;

        glm::vec2 advance{ 0.0f };
        const size_t length = text.caption.size();
        for (size_t i = 0; i < length; ++i)
        {
            const unicode_t character = text.caption[i];

            if (i > 0)
            {
                const unicode_t prev_character = text.caption[i - 1];

                const std::pair<unicode_t, unicode_t> key = std::make_pair(prev_character, character);
                if (font.kerning.contains(key))
                {
                    advance.x += font.kerning.at(key) * atlas.font_size;
                }

                if (font.glyphs.contains(prev_character))
                {
                    advance.x += font.glyphs.at(prev_character).advance * atlas.font_size;
                }
            }

            if (character == ' ')
            {
                advance.x += font.metrics.em_size * atlas.font_size / 4.0f;
            }
            else if (character == '\t')
            {
                advance.x += font.metrics.em_size * atlas.font_size * 2.0f;
            }
            else if (character == '\n')
            {
                advance.x = 0.0f;
                advance.y -= font.metrics.line_height * atlas.font_size;
            }
            else if (font.glyphs.contains(character))
            {
                const Font::GlyphData& glyph = font.glyphs.at(character);

                const Font::GlyphData::Bounds& atlas_bounds = glyph.atlas_bounds;
                const Font::GlyphData::Bounds& plane_bounds = glyph.plane_bounds;

                const glm::vec2 position = advance - glm::vec2{ plane_bounds.left, plane_bounds.bottom } * atlas.font_size;

                const glm::vec2 u_vector = glm::vec2{ atlas_bounds.left, atlas_bounds.bottom } / uv_length;
                const glm::vec2 v_vector = glm::vec2{ atlas_bounds.right, atlas_bounds.top } / uv_length;

                const float width = atlas_bounds.right - atlas_bounds.left;
                const float height = atlas_bounds.top - atlas_bounds.bottom;

                Vertex v;
                v.position = transformation * glm::vec4{ position, 0.0, 1.0f };
                v.tex_id = atlas.origin.id;
                v.tex_coord = glm::vec2{ u_vector.x, v_vector.y };
                v.color = color;
                v.additional = screen_px_range;
                key.depth = v.position.z;
                _render_queue.emplace(key, v);

                v.position = transformation * glm::vec4{ position.x + width, position.y, 0.0f, 1.0f };
                v.tex_id = atlas.origin.id;
                v.tex_coord = v_vector;
                v.color = color;
                v.additional = screen_px_range;
                key.depth = v.position.z;
                _render_queue.emplace(key, v);

                v.position = transformation * glm::vec4{ position.x + width, position.y + height, 0.0f, 1.0f };
                v.tex_id = atlas.origin.id;
                v.tex_coord = glm::vec2{ v_vector.x, u_vector.y };
                v.color = color;
                v.additional = screen_px_range;
                key.depth = v.position.z;
                _render_queue.emplace(key, v);

                v.position = transformation * glm::vec4{ position.x, position.y + height, 0.0f, 1.0f };
                v.tex_id = atlas.origin.id;
                v.tex_coord = u_vector;
                v.color = color;
                v.additional = screen_px_range;
                key.depth = v.position.z;
                _render_queue.emplace(key, v);
            }
        }
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
