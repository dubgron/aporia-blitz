#include "renderer.hpp"

#include <array>
#include <cmath>
#include <numeric>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "common.hpp"
#include "event_manager.hpp"
#include "shader_manager.hpp"
#include "window.hpp"
#include "configs/window_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/drawables/circle2d.hpp"
#include "graphics/drawables/group.hpp"
#include "graphics/drawables/line2d.hpp"
#include "graphics/drawables/rectangle2d.hpp"
#include "graphics/drawables/sprite.hpp"
#include "graphics/drawables/text.hpp"
#include "platform/opengl.hpp"
#include "utils/math.hpp"

#if defined(APORIA_DEBUG)
    #include <imgui.h>
    #define DEBUG_TEXTURE(texture) \
        ImGui::Begin("DEBUG | Textures");\
        ImGui::Text("ID: %d, Size: %d x %d", texture.id, texture.width, texture.height); \
        ImGui::Image((void*)(intptr_t)texture.id, ImVec2{ (float)texture.width, (float)texture.height }, ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f }); \
        ImGui::End();
#else
    #define DEBUG_TEXTURE
#endif

namespace Aporia
{
    ShaderRef Renderer::default_shader = 0;
    ShaderRef Renderer::font_shader = 0;
    ShaderRef Renderer::postprocessing_shader = 0;

    Renderer::Renderer(ShaderManager& shaders, EventManager& events, WindowConfig& config)
        : _shaders(shaders)
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
        lights.masking.create_framebuffer(config.width, config.height);
        lights.raymarching.create_framebuffer(config.width, config.height);

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

        /* Setup lighting shaders */
        lights.raymarching_shader = _shaders.create_program("raymarching", "assets/shaders/raymarching.shader");
        lights.shadowcasting_shader = _shaders.create_program("shadowcasting", "assets/shaders/shadowcasting.shader");

        _shaders.unbind();

        /* Bind Light's Uniform Buffer to shaders */
        lights.uniform_buffer.bind_to_shader(lights.raymarching_shader);
        lights.uniform_buffer.bind_to_shader(lights.shadowcasting_shader);

        /* Bind to events */
        using namespace std::placeholders;

        events.add_listener<WindowResizeEvent>( std::bind(&Renderer::_on_resize, this, _1, _2, _3) );
    }

    void Renderer::begin(const Window& window, const Camera& camera)
    {
        lights.begin();

        _shaders.bind(default_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());

        _shaders.bind(font_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        _shaders.set_float("u_camera_zoom", 1.0f / camera.get_zoom());

        _shaders.bind(lights.raymarching_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        _shaders.set_int("u_masking", lights.masking.get_color_buffer().id);
        _shaders.set_float("u_camera_zoom", 1.0f / camera.get_zoom());
        _shaders.set_float2("u_window_size", glm::vec2{ window.get_size() });

        _shaders.bind(lights.shadowcasting_shader);
        _shaders.set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        _shaders.set_int("u_raymarching", lights.raymarching.get_color_buffer().id);
        _shaders.set_float("u_camera_zoom", 1.0f / camera.get_zoom());
        _shaders.set_float2("u_window_size", glm::vec2{ window.get_size() });

        _shaders.unbind();
    }

    void Renderer::end(Color color /* = Colors::Black */)
    {
        _framebuffer.bind();
        _framebuffer.clear(color);

        _flush_queue();

        _framebuffer.unbind();

        DEBUG_TEXTURE(_framebuffer.get_color_buffer());

        _flush_framebuffer(_framebuffer, postprocessing_shader);

        if (lights.enabled)
        {
            lights.end();

            const uint32_t num_lights = static_cast<uint32_t>(lights.sources.size());

            _shaders.bind(lights.raymarching_shader);
            _shaders.set_uint("u_num_lights", num_lights);

            _shaders.bind(lights.shadowcasting_shader);
            _shaders.set_uint("u_num_lights", num_lights);

            lights.masking.bind();
            lights.masking.clear(Colors::Transparent);

            draw(lights.blockers);
            _flush_queue();

            lights.masking.unbind();

            DEBUG_TEXTURE(lights.masking.get_color_buffer());

            lights.raymarching.bind();
            lights.raymarching.clear(Colors::Black);

            _flush_framebuffer(lights.masking, lights.raymarching_shader);

            lights.raymarching.unbind();

            DEBUG_TEXTURE(lights.raymarching.get_color_buffer());

            _flush_framebuffer(lights.raymarching, lights.shadowcasting_shader);
        }

        _shaders.unbind();
    }

    void Renderer::draw(const Group& group)
    {
        push_transform(group.transform);

        for (const Line2D& line : group.get_container<Line2D>())
        {
            draw(line);
        }

        for (const Rectangle2D& rectangle : group.get_container<Rectangle2D>())
        {
            draw(rectangle);
        }

        for (const Sprite& sprite : group.get_container<Sprite>())
        {
            draw(sprite);
        }

        for (const Text& text : group.get_container<Text>())
        {
            draw(text);
        }

        for (const Group& group : group.get_container<Group>())
        {
            draw(group);
        }

        pop_transform();
    }

    void Renderer::draw(const Sprite& sprite)
    {
        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * sprite.transform);

        const glm::vec3 base_offset     = transformation[3];
        const glm::vec3 right_offset    = transformation[0] * sprite.rect.width;
        const glm::vec3 up_offset       = transformation[1] * sprite.rect.height;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = sprite.shader;

        key.vertex[0].position      = base_offset;
        key.vertex[0].tex_coord     = glm::vec2{ sprite.texture.u.x, sprite.texture.v.y };
        key.vertex[0].tex_id        = sprite.texture.origin.id;
        key.vertex[0].color         = sprite.color;

        key.vertex[1].position      = base_offset + right_offset;
        key.vertex[1].tex_coord     = sprite.texture.v;
        key.vertex[1].tex_id        = sprite.texture.origin.id;
        key.vertex[1].color         = sprite.color;

        key.vertex[2].position      = base_offset + right_offset + up_offset;
        key.vertex[2].tex_coord     = glm::vec2{ sprite.texture.v.x, sprite.texture.u.y };
        key.vertex[2].tex_id        = sprite.texture.origin.id;
        key.vertex[2].color         = sprite.color;

        key.vertex[3].position      = base_offset + up_offset;
        key.vertex[3].tex_coord     = sprite.texture.u;
        key.vertex[3].tex_id        = sprite.texture.origin.id;
        key.vertex[3].color         = sprite.color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Rectangle2D& rect)
    {
        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * rect.transform);

        const glm::vec3 base_offset     = transformation[3];
        const glm::vec3 right_offset    = transformation[0] * rect.size.width;
        const glm::vec3 up_offset       = transformation[1] * rect.size.height;

        RenderQueueKey key;
        key.buffer              = BufferType::Quads;
        key.program_id          = rect.shader;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = rect.color;
        key.vertex[0].tex_coord = glm::vec2{ 0.0f, 0.0f };

        key.vertex[1].position  = base_offset + right_offset;
        key.vertex[1].color     = rect.color;
        key.vertex[1].tex_coord = glm::vec2{ 1.0f, 0.0f };

        key.vertex[2].position  = base_offset + right_offset + up_offset;
        key.vertex[2].color     = rect.color;
        key.vertex[2].tex_coord = glm::vec2{ 1.0f, 1.0f };

        key.vertex[3].position  = base_offset + up_offset;
        key.vertex[3].color     = rect.color;
        key.vertex[3].tex_coord = glm::vec2{ 0.0f, 1.0f };

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Line2D& line2d)
    {
        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * line2d.transform);

        const glm::vec3 base_offset = transformation[3];
        const glm::vec3 offset      = glm::mat2x4{ transformation[0], transformation[1] } * line2d.offset;

        RenderQueueKey key;
        key.buffer              = BufferType::Lines;
        key.program_id          = line2d.shader;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = line2d.color;

        key.vertex[1].position  = base_offset + offset;
        key.vertex[1].color     = line2d.color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Circle2D& circle)
    {
        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * circle.transform);

        const glm::vec3 base_offset          = transformation[3];
        const glm::vec3 right_half_offset    = transformation[0] * circle.radius;
        const glm::vec3 up_half_offset       = transformation[1] * circle.radius;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = circle.shader;

        key.vertex[0].position      = base_offset - right_half_offset - up_half_offset;
        key.vertex[0].color         = circle.color;
        key.vertex[0].tex_coord     = glm::vec2{ -1.0f, -1.0f };

        key.vertex[1].position      = base_offset + right_half_offset - up_half_offset;
        key.vertex[1].color         = circle.color;
        key.vertex[1].tex_coord     = glm::vec2{ 1.0f, -1.0f };

        key.vertex[2].position      = base_offset + right_half_offset + up_half_offset;
        key.vertex[2].color         = circle.color;
        key.vertex[2].tex_coord     = glm::vec2{ 1.0f, 1.0f };

        key.vertex[3].position      = base_offset - right_half_offset + up_half_offset;
        key.vertex[3].color         = circle.color;
        key.vertex[3].tex_coord     = glm::vec2{ -1.0f, 1.0f };

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Text& text)
    {
        if (!text.font)
        {
            return;
        }

        const Font& font = *text.font;

        const glm::vec2 uv_length = glm::vec2{ font.atlas.origin.width, font.atlas.origin.height };

        /* Adjust text scaling by the predefined atlas font size */
        const Transform2D font_scale{ .scale = glm::vec2{ 1.0f / font.atlas.font_size } };

        const glm::mat4 transformation = to_mat4(_transformation_stack.back() * text.transform * font_scale);

        const float text_scale = glm::length(text.transform.scale);
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
                    advance.x += font.kerning.at(key) * font.atlas.font_size;
                }

                if (font.glyphs.contains(prev_character))
                {
                    advance.x += font.glyphs.at(prev_character).advance * font.atlas.font_size;
                }
            }

            if (character == ' ')
            {
                advance.x += font.metrics.em_size * font.atlas.font_size / 4.0f;
            }
            else if (character == '\t')
            {
                advance.x += font.metrics.em_size * font.atlas.font_size * 2.0f;
            }
            else if (character == '\n')
            {
                advance.x = 0.0f;
                advance.y -= font.metrics.line_height * font.atlas.font_size;
            }
            else if (font.glyphs.contains(character))
            {
                const Font::GlyphData& glyph = font.glyphs.at(character);

                const Font::GlyphData::Bounds& atlas_bounds = glyph.atlas_bounds;
                const Font::GlyphData::Bounds& plane_bounds = glyph.plane_bounds;

                const glm::vec2 position = advance - glm::vec2{ plane_bounds.left, plane_bounds.bottom } * font.atlas.font_size;

                const glm::vec2 u_vector = glm::vec2{ atlas_bounds.left, atlas_bounds.bottom } / uv_length;
                const glm::vec2 v_vector = glm::vec2{ atlas_bounds.right, atlas_bounds.top } / uv_length;

                const float width = atlas_bounds.right - atlas_bounds.left;
                const float height = atlas_bounds.top - atlas_bounds.bottom;

                RenderQueueKey key;
                key.buffer                  = BufferType::Quads;
                key.program_id              = text.shader;

                key.vertex[0].position      = transformation * glm::vec4{ position, 0.0f, 1.0f };
                key.vertex[0].tex_id        = font.atlas.origin.id;
                key.vertex[0].tex_coord     = glm::vec2{ u_vector.x, v_vector.y };
                key.vertex[0].color         = text.color;
                key.vertex[0].additional    = screen_px_range;

                key.vertex[1].position      = transformation * glm::vec4{ position.x + width, position.y, 0.0f, 1.0f };
                key.vertex[1].tex_id        = font.atlas.origin.id;
                key.vertex[1].tex_coord     = v_vector;
                key.vertex[1].color         = text.color;
                key.vertex[1].additional    = screen_px_range;

                key.vertex[2].position      = transformation * glm::vec4{ position.x + width, position.y + height, 0.0f, 1.0f };
                key.vertex[2].tex_id        = font.atlas.origin.id;
                key.vertex[2].tex_coord     = glm::vec2{ v_vector.x, u_vector.y };
                key.vertex[2].color         = text.color;
                key.vertex[2].additional    = screen_px_range;

                key.vertex[3].position      = transformation * glm::vec4{ position.x, position.y + height, 0.0f, 1.0f };
                key.vertex[3].tex_id        = font.atlas.origin.id;
                key.vertex[3].tex_coord     = u_vector;
                key.vertex[3].color         = text.color;
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

    void Renderer::_flush_queue()
    {
        if (!_render_queue.empty())
        {
            std::sort(_render_queue.begin(), _render_queue.end());

            RenderQueueKey prev_key = _render_queue[0];
            for (RenderQueueKey& key : _render_queue)
            {
                if (key.program_id != prev_key.program_id || key.buffer != prev_key.buffer)
                {
                    _flush_buffer(prev_key.buffer, prev_key.program_id);
                }

                const uint8_t buffer_index = std::to_underlying(key.buffer);
                VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
                for (size_t i = 0; i < vertex_buffer.count(); ++i)
                {
                    vertex_buffer.emplace( std::move(key.vertex[i]) );
                }

                prev_key = std::move(key);
            }

            _flush_buffer(prev_key.buffer, prev_key.program_id);

            _render_queue.clear();
        }
    }

    void Renderer::_flush_framebuffer(const Framebuffer& framebuffer, Shader program_id)
    {
        constexpr uint8_t buffer_index = std::to_underlying(BufferType::Quads);
        VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
        for (Vertex vertex : framebuffer.get_vertices())
        {
            vertex_buffer.emplace( std::move(vertex) );
        }

        _flush_buffer(BufferType::Quads, program_id);
    }

    void Renderer::_flush_buffer(BufferType buffer, Shader program_id)
    {
        _shaders.bind(program_id);

        const uint8_t buffer_index = std::to_underlying(buffer);
        _vertex_arrays[buffer_index].render();
    }

    void Renderer::_on_resize(Window& window, uint32_t width, uint32_t height)
    {
        _framebuffer.create_framebuffer(width, height);
        lights.masking.create_framebuffer(width, height);
        lights.raymarching.create_framebuffer(width, height);
    }
}
