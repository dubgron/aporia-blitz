#include "renderer.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "utils/math.hpp"

namespace Aporia
{
    ShaderRef Renderer::default_shader = 0u;
    ShaderRef Renderer::font_shader = 0u;
    ShaderRef Renderer::postprocessing_shader = 0u;

    Renderer::Renderer(Logger& logger, ShaderManager& shaders, EventManager& events, WindowConfig& config)
        : _logger(logger), _shaders(shaders)
    {
        _transformation_stack.reserve(10);
        _transformation_stack.emplace_back();

        _render_queue.reserve(MAX_QUEUE);

        _vertex_arrays.reserve(2);

        /* Set VertexArray for opaque Quads */
        VertexArray& quads = _vertex_arrays.emplace_back(4, 6);
        quads.bind();

        VertexBuffer quads_vbo{ MAX_QUEUE, 4 };
        quads_vbo.add_layout();
        quads.set_vertex_buffer( std::move(quads_vbo) );

        std::vector<uint32_t> quad_indecies(MAX_QUEUE * 6);
        for (auto [i, offset] = std::make_pair<size_t, uint32_t>(0, 0); i < MAX_QUEUE * 6; i += 6, offset += 4)
        {
            quad_indecies[  i  ] = offset + 0;
            quad_indecies[i + 1] = offset + 1;
            quad_indecies[i + 2] = offset + 2;

            quad_indecies[i + 3] = offset + 2;
            quad_indecies[i + 4] = offset + 3;
            quad_indecies[i + 5] = offset + 0;
        }

        IndexBuffer quads_ibo{ MAX_QUEUE, 6, quad_indecies };
        quads.set_index_buffer( std::move(quads_ibo) );

        quads.unbind();

        /* Set VertexArray for Lines */
        VertexArray& lines = _vertex_arrays.emplace_back(2, 2);
        lines.bind();

        VertexBuffer lines_vbo{ MAX_QUEUE, 2 };
        lines_vbo.add_layout();
        lines.set_vertex_buffer( std::move(lines_vbo) );

        std::vector<uint32_t> line_indecies(MAX_QUEUE * 2);
        for (uint32_t i = 0; i < MAX_QUEUE * 2; ++i)
        {
            line_indecies[i] = i;
        }

        IndexBuffer lines_ibo{ MAX_QUEUE, 2, line_indecies };
        lines.set_index_buffer( std::move(lines_ibo) );

        lines.unbind();

        /* Setup Framebuffer */
        _framebuffer.create_framebuffer(config.width, config.height);

        /* Initialize texture sampler */
        std::array<int32_t, OPENGL_MAX_TEXTURE_UNITS> sampler{};
        std::iota(sampler.begin(), sampler.end(), 0);

        /* Setup default shaders */
        default_shader = _shaders.create_program("default", "assets/shaders/default.shader");

        _shaders.bind(default_shader);
        _shaders.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);

        /* Setup font shaders */
        font_shader = _shaders.create_program("font", "assets/shaders/font.shader");

        _shaders.bind(font_shader);
        _shaders.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);

        /* Setup post-processing shaders */
        postprocessing_shader = _shaders.create_program("post-processing", "assets/shaders/postprocessing.shader");

        _shaders.bind(postprocessing_shader);
        _shaders.set_int_array("u_atlas", &sampler[0], OPENGL_MAX_TEXTURE_UNITS);

        _shaders.unbind();

        /* Bind to events */
        using namespace std::placeholders;

        events.add_listener<WindowResizeEvent>( std::bind(&Renderer::_on_resize, this, _1, _2, _3) );
    }

    void Renderer::begin(Camera& camera)
    {
        _shaders.bind(default_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());

        _shaders.bind(font_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        _shaders.set_float("u_camera_zoom", 1.0f / camera.get_zoom());

        /* Bind Framebuffer */
        _framebuffer.bind();
    }

    void Renderer::end()
    {
        if (!_render_queue.empty())
        {
            std::sort(_render_queue.begin(), _render_queue.end());

            RenderQueueKey prev_key = _render_queue[0];
            for (RenderQueueKey& key : _render_queue)
            {
                if (key.program_id != prev_key.program_id || key.buffer != prev_key.buffer)
                {
                    flush(prev_key.program_id, prev_key.buffer);
                }

                const uint8_t buffer_index = std::to_underlying(key.buffer);
                VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
                for (size_t i = 0; i < vertex_buffer.vertex_count(); ++i)
                {
                    vertex_buffer.emplace( std::move(key.vertex[i]) );
                }

                prev_key = std::move(key);
            }

            flush(prev_key.program_id, prev_key.buffer);

            _render_queue.clear();
        }

        /* Flush Framebuffer */
        _framebuffer.unbind();

        constexpr uint8_t buffer_index = std::to_underlying(BufferType::Quads);
        VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
        for (Vertex vertex : _framebuffer.get_vertices())
        {
            vertex_buffer.emplace( std::move(vertex) );
        }

        flush(postprocessing_shader, BufferType::Quads);

        _shaders.unbind();
    }

    void Renderer::flush(Shader program_id, BufferType buffer)
    {
        _shaders.bind(program_id);

        const uint8_t buffer_index = std::to_underlying(buffer);
        _vertex_arrays[buffer_index].render();
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

    void Renderer::draw(const Sprite& sprite, Shader program_id /* = default_shader */)
    {
        const Transform2D& transform = sprite.get_component<Transform2D>();
        const Rectangular& rectangular = sprite.get_component<Rectangular>();
        const Texture& texture = sprite.get_component<Texture>();
        const Color& color = sprite.get_component<Color>();

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * transform);

        const glm::vec3 base_offset     = transformation[3];
        const glm::vec3 right_offset    = transformation[0] * rectangular.width;
        const glm::vec3 up_offset       = transformation[1] * rectangular.height;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = program_id;

        key.vertex[0].position      = base_offset;
        key.vertex[0].tex_coord     = glm::vec2{ texture.u.x, texture.v.y };
        key.vertex[0].tex_id        = texture.origin.id;
        key.vertex[0].color         = color;

        key.vertex[1].position      = base_offset + right_offset;
        key.vertex[1].tex_coord     = texture.v;
        key.vertex[1].tex_id        = texture.origin.id;
        key.vertex[1].color         = color;

        key.vertex[2].position      = base_offset + right_offset + up_offset;
        key.vertex[2].tex_coord     = glm::vec2{ texture.v.x, texture.u.y };
        key.vertex[2].tex_id        = texture.origin.id;
        key.vertex[2].color         = color;

        key.vertex[3].position      = base_offset + up_offset;
        key.vertex[3].tex_coord     = texture.u;
        key.vertex[3].tex_id        = texture.origin.id;
        key.vertex[3].color         = color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Rectangle2D& rect, Shader program_id /* = default_shader */)
    {
        const Transform2D& transform = rect.get_component<Transform2D>();
        const Rectangular& rectangular = rect.get_component<Rectangular>();
        const Color& color = rect.get_component<Color>();

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * transform);

        const glm::vec3 base_offset     = transformation[3];
        const glm::vec3 right_offset    = transformation[0] * rectangular.width;
        const glm::vec3 up_offset       = transformation[1] * rectangular.height;

        RenderQueueKey key;
        key.buffer              = BufferType::Quads;
        key.program_id          = program_id;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = color;

        key.vertex[1].position  = base_offset + right_offset;
        key.vertex[1].color     = color;

        key.vertex[2].position  = base_offset + right_offset + up_offset;
        key.vertex[2].color     = color;

        key.vertex[3].position  = base_offset + up_offset;
        key.vertex[3].color     = color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Line2D& line2d, Shader program_id /* = default_shader */)
    {
        const Transform2D& transform = line2d.get_component<Transform2D>();
        const Linear2D& linear2d = line2d.get_component<Linear2D>();
        const Color& color = line2d.get_component<Color>();

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * transform);

        const glm::vec3 base_offset = transformation[3];
        const glm::vec3 offset      = glm::mat2x4{ transformation[0], transformation[1] } * linear2d.point;

        RenderQueueKey key;
        key.buffer              = BufferType::Lines;
        key.program_id          = program_id;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = color;

        key.vertex[1].position  = base_offset + offset;
        key.vertex[1].color     = color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Circle2D& circle, Shader program_id /* = default_shader */)
    {
        const Transform2D& transform = circle.get_component<Transform2D>();
        const Circular& circular = circle.get_component<Circular>();
        const Color& color = circle.get_component<Color>();

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * transform);

        const glm::vec3 base_offset          = transformation[3];
        const glm::vec3 right_half_offset    = transformation[0] * circular.radius;
        const glm::vec3 up_half_offset       = transformation[1] * circular.radius;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = program_id;

        key.vertex[0].position      = base_offset - right_half_offset - up_half_offset;
        key.vertex[0].color         = color;
        key.vertex[0].tex_coord     = glm::vec2{ -1.0f, -1.0f };

        key.vertex[1].position      = base_offset + right_half_offset - up_half_offset;
        key.vertex[1].color         = color;
        key.vertex[1].tex_coord     = glm::vec2{ 1.0f, -1.0f };

        key.vertex[2].position      = base_offset + right_half_offset + up_half_offset;
        key.vertex[2].color         = color;
        key.vertex[2].tex_coord     = glm::vec2{ 1.0f, 1.0f };

        key.vertex[3].position      = base_offset - right_half_offset + up_half_offset;
        key.vertex[3].color         = color;
        key.vertex[3].tex_coord     = glm::vec2{ -1.0f, 1.0f };

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Text& text, Shader program_id /* = font_shader */)
    {
        const Transform2D& transform = text.get_component<Transform2D>();
        const Color& color = text.get_component<Color>();

        const Font& font = text.font;
        const Font::Atlas& atlas = font.atlas;

        const glm::vec2 uv_length = glm::vec2{ atlas.origin.width, atlas.origin.height };

        /* Adjust text scaling by the predefined atlas font size */
        const Transform2D font_scale{ .scale = glm::vec2{ 1.0f / atlas.font_size } };

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * transform * font_scale);

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

                RenderQueueKey key;
                key.buffer                  = BufferType::Quads;
                key.program_id              = program_id;

                key.vertex[0].position      = transformation * glm::vec4{ position, 0.0f, 1.0f };
                key.vertex[0].tex_id        = atlas.origin.id;
                key.vertex[0].tex_coord     = glm::vec2{ u_vector.x, v_vector.y };
                key.vertex[0].color         = color;
                key.vertex[0].additional    = screen_px_range;

                key.vertex[1].position      = transformation * glm::vec4{ position.x + width, position.y, 0.0f, 1.0f };
                key.vertex[1].tex_id        = atlas.origin.id;
                key.vertex[1].tex_coord     = v_vector;
                key.vertex[1].color         = color;
                key.vertex[1].additional    = screen_px_range;

                key.vertex[2].position      = transformation * glm::vec4{ position.x + width, position.y + height, 0.0f, 1.0f };
                key.vertex[2].tex_id        = atlas.origin.id;
                key.vertex[2].tex_coord     = glm::vec2{ v_vector.x, u_vector.y };
                key.vertex[2].color         = color;
                key.vertex[2].additional    = screen_px_range;

                key.vertex[3].position      = transformation * glm::vec4{ position.x, position.y + height, 0.0f, 1.0f };
                key.vertex[3].tex_id        = atlas.origin.id;
                key.vertex[3].tex_coord     = u_vector;
                key.vertex[3].color         = color;
                key.vertex[3].additional    = screen_px_range;

                _render_queue.push_back( std::move(key) );
            }
        }
    }

    void Renderer::push_transform(const Transform2D& transform)
    {
        _transformation_stack.push_back( _transformation_stack.back() * transform );
    }

    void Renderer::pop_transform()
    {
        if (_transformation_stack.size() > 1)
        {
            _transformation_stack.pop_back();
        }
    }

    void Renderer::_on_resize(Window& window, uint32_t width, uint32_t height)
    {
        _framebuffer.create_framebuffer(width, height);
    }
}
