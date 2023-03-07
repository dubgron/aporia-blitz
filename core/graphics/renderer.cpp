#include "renderer.hpp"

#include <array>
#include <cmath>
#include <numeric>

#include "aporia_shaders.hpp"
#include "aporia_types.hpp"
#include "common.hpp"
#include "window.hpp"
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
        ImGui::Image((void*)(intptr_t)texture.id, ImVec2{ (f32)texture.width, (f32)texture.height }, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }); \
        ImGui::End();
#else
    #define DEBUG_TEXTURE
#endif

namespace Aporia
{
    ShaderID Renderer::default_shader = 0;
    ShaderID Renderer::font_shader = 0;
    ShaderID Renderer::postprocessing_shader = 0;

    void Renderer::init(u32 width, u32 height)
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

        std::vector<u32> quad_indices(MAX_QUEUE * 6);
        for (auto [i, offset] = std::make_pair<u64, u32>(0, 0); i < MAX_QUEUE * 6; i += 6, offset += 4)
        {
            quad_indices[  i  ] = offset + 0;
            quad_indices[i + 1] = offset + 1;
            quad_indices[i + 2] = offset + 2;

            quad_indices[i + 3] = offset + 2;
            quad_indices[i + 4] = offset + 3;
            quad_indices[i + 5] = offset + 0;
        }

        IndexBuffer quads_ibo{ MAX_QUEUE, 6, quad_indices };
        quads.set_index_buffer( std::move(quads_ibo) );

        quads.unbind();

        /* Set VertexArray for Lines */
        VertexArray& lines = _vertex_arrays.emplace_back(2, 2);
        lines.bind();

        VertexBuffer lines_vbo{ MAX_QUEUE, 2 };
        lines_vbo.add_layout();
        lines.set_vertex_buffer( std::move(lines_vbo) );

        std::vector<u32> line_indices(MAX_QUEUE * 2);
        for (u32 i = 0; i < MAX_QUEUE * 2; ++i)
        {
            line_indices[i] = i;
        }

        IndexBuffer lines_ibo{ MAX_QUEUE, 2, line_indices };
        lines.set_index_buffer( std::move(lines_ibo) );

        lines.unbind();

        /* Initialize Light Renderer */
        lights.init();

        /* Setup Framebuffer */
        _framebuffer.create_framebuffer(width, height);
        lights.masking.create_framebuffer(width, height);
        lights.raymarching.create_framebuffer(width, height);

        /* Initialize texture sampler */
        std::array<i32, OPENGL_MAX_TEXTURE_UNITS> sampler{};
        std::iota(sampler.begin(), sampler.end(), 0);

        /* Setup default shaders */
        default_shader = create_shader("assets/shaders/default.shader");

        bind_shader(default_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        /* Setup font shaders */
        font_shader = create_shader("assets/shaders/font.shader");

        bind_shader(font_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        /* Setup post-processing shaders */
        postprocessing_shader = create_shader("assets/shaders/postprocessing.shader");

        bind_shader(postprocessing_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        /* Setup lighting shaders */
        lights.raymarching_shader = create_shader("assets/shaders/raymarching.shader");
        lights.shadowcasting_shader = create_shader("assets/shaders/shadowcasting.shader");

        unbind_shader();

        /* Bind Light's Uniform Buffer to shaders */
        lights.uniform_buffer.bind_to_shader(lights.raymarching_shader);
        lights.uniform_buffer.bind_to_shader(lights.shadowcasting_shader);
    }

    void Renderer::deinit()
    {
        lights.deinit();
        remove_all_shaders();
    }

    void Renderer::begin(const Window& window, const Camera& camera)
    {
        lights.begin();

        bind_shader(default_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());

        bind_shader(font_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());

        bind_shader(lights.raymarching_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_int("u_masking", lights.masking.get_color_buffer().id);
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());
        shader_set_float2("u_window_size", v2{ window.get_size() });

        bind_shader(lights.shadowcasting_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_int("u_raymarching", lights.raymarching.get_color_buffer().id);
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());
        shader_set_float2("u_window_size", v2{ window.get_size() });

        unbind_shader();
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

            const u32 num_lights = static_cast<u32>(lights.sources.size());

            bind_shader(lights.raymarching_shader);
            shader_set_uint("u_num_lights", num_lights);

            bind_shader(lights.shadowcasting_shader);
            shader_set_uint("u_num_lights", num_lights);

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

        unbind_shader();
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
        const m4 transformation = to_mat4(_transformation_stack.back() * sprite.transform);

        const v3 base_offset     = transformation[3];
        const v3 right_offset    = transformation[0] * sprite.rect.width;
        const v3 up_offset       = transformation[1] * sprite.rect.height;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = sprite.shader;

        const v2 half_pixel_offset = 0.5f / v2{ sprite.texture.source.width, sprite.texture.source.height };
        const v2 tex_coord_u = sprite.texture.u + half_pixel_offset;
        const v2 tex_coord_v = sprite.texture.v - half_pixel_offset;

        key.vertex[0].position      = base_offset;
        key.vertex[0].tex_coord     = v2{ tex_coord_u.x, tex_coord_v.y };
        key.vertex[0].tex_id        = sprite.texture.source.id;
        key.vertex[0].color         = sprite.color;

        key.vertex[1].position      = base_offset + right_offset;
        key.vertex[1].tex_coord     = tex_coord_v;
        key.vertex[1].tex_id        = sprite.texture.source.id;
        key.vertex[1].color         = sprite.color;

        key.vertex[2].position      = base_offset + right_offset + up_offset;
        key.vertex[2].tex_coord     = v2{ tex_coord_v.x, tex_coord_u.y };
        key.vertex[2].tex_id        = sprite.texture.source.id;
        key.vertex[2].color         = sprite.color;

        key.vertex[3].position      = base_offset + up_offset;
        key.vertex[3].tex_coord     = tex_coord_u;
        key.vertex[3].tex_id        = sprite.texture.source.id;
        key.vertex[3].color         = sprite.color;

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Rectangle2D& rect)
    {
        const m4 transformation = to_mat4(_transformation_stack.back() * rect.transform);

        const v3 base_offset     = transformation[3];
        const v3 right_offset    = transformation[0] * rect.size.width;
        const v3 up_offset       = transformation[1] * rect.size.height;

        RenderQueueKey key;
        key.buffer              = BufferType::Quads;
        key.program_id          = rect.shader;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = rect.color;
        key.vertex[0].tex_coord = v2{ 0.f, 0.f };

        key.vertex[1].position  = base_offset + right_offset;
        key.vertex[1].color     = rect.color;
        key.vertex[1].tex_coord = v2{ 1.f, 0.f };

        key.vertex[2].position  = base_offset + right_offset + up_offset;
        key.vertex[2].color     = rect.color;
        key.vertex[2].tex_coord = v2{ 1.f, 1.f };

        key.vertex[3].position  = base_offset + up_offset;
        key.vertex[3].color     = rect.color;
        key.vertex[3].tex_coord = v2{ 0.f, 1.f };

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Line2D& line2d)
    {
        const m4 transformation = to_mat4(_transformation_stack.back() * line2d.transform);

        const v3 base_offset = transformation[3];
        const v3 offset      = glm::mat2x4{ transformation[0], transformation[1] } * line2d.offset;

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
        const m4 transformation = to_mat4(_transformation_stack.back() * circle.transform);

        const v3 base_offset          = transformation[3];
        const v3 right_half_offset    = transformation[0] * circle.radius;
        const v3 up_half_offset       = transformation[1] * circle.radius;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.program_id              = circle.shader;

        key.vertex[0].position      = base_offset - right_half_offset - up_half_offset;
        key.vertex[0].color         = circle.color;
        key.vertex[0].tex_coord     = v2{ -1.f, -1.f };

        key.vertex[1].position      = base_offset + right_half_offset - up_half_offset;
        key.vertex[1].color         = circle.color;
        key.vertex[1].tex_coord     = v2{ 1.f, -1.f };

        key.vertex[2].position      = base_offset + right_half_offset + up_half_offset;
        key.vertex[2].color         = circle.color;
        key.vertex[2].tex_coord     = v2{ 1.f, 1.f };

        key.vertex[3].position      = base_offset - right_half_offset + up_half_offset;
        key.vertex[3].color         = circle.color;
        key.vertex[3].tex_coord     = v2{ -1.f, 1.f };

        _render_queue.push_back( std::move(key) );
    }

    void Renderer::draw(const Text& text)
    {
        if (!text.font)
        {
            return;
        }

        const Font& font = *text.font;

        const v2 texture_size = v2{ font.atlas.source.width, font.atlas.source.height };
        const v2 half_pixel_offset = 0.5f / texture_size;

        /* Adjust text scaling by the predefined atlas font size */
        const Transform2D font_scale{ .scale = v2{ 1.f / font.atlas.font_size } };

        const m4 transformation = to_mat4(_transformation_stack.back() * text.transform * font_scale);

        const f32 text_scale = glm::length(text.transform.scale);
        const f32 screen_px_range = text_scale * font.atlas.distance_range / font.atlas.font_size;

        v2 advance{ 0.f };
        const u64 length = text.caption.size();
        for (u64 i = 0; i < length; ++i)
        {
            const u8 character = text.caption[i];

            if (i > 0)
            {
                const u8 prev_character = text.caption[i - 1];

                const std::pair<u8, u8> key = std::make_pair(prev_character, character);
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
                advance.x += font.metrics.em_size * font.atlas.font_size / 4.f;
            }
            else if (character == '\t')
            {
                advance.x += font.metrics.em_size * font.atlas.font_size * 2.f;
            }
            else if (character == '\n')
            {
                advance.x = 0.f;
                advance.y -= font.metrics.line_height * font.atlas.font_size;
            }
            else if (font.glyphs.contains(character))
            {
                const Glyph& glyph = font.glyphs.at(character);

                const GlyphBounds& atlas_bounds = glyph.atlas_bounds;
                const GlyphBounds& plane_bounds = glyph.plane_bounds;

                const v2 position = advance - v2{ plane_bounds.left, plane_bounds.bottom } * font.atlas.font_size;

                const v2 tex_coord_u = v2{ atlas_bounds.left, atlas_bounds.bottom } / texture_size + half_pixel_offset;
                const v2 tex_coord_v = v2{ atlas_bounds.right, atlas_bounds.top } / texture_size - half_pixel_offset;

                const f32 width = atlas_bounds.right - atlas_bounds.left;
                const f32 height = atlas_bounds.top - atlas_bounds.bottom;

                RenderQueueKey key;
                key.buffer                  = BufferType::Quads;
                key.program_id              = text.shader;

                key.vertex[0].position      = transformation * v4{ position, 0.f, 1.f };
                key.vertex[0].tex_id        = font.atlas.source.id;
                key.vertex[0].tex_coord     = v2{ tex_coord_u.x, tex_coord_v.y };
                key.vertex[0].color         = text.color;
                key.vertex[0].additional    = screen_px_range;

                key.vertex[1].position      = transformation * v4{ position.x + width, position.y, 0.f, 1.f };
                key.vertex[1].tex_id        = font.atlas.source.id;
                key.vertex[1].tex_coord     = tex_coord_v;
                key.vertex[1].color         = text.color;
                key.vertex[1].additional    = screen_px_range;

                key.vertex[2].position      = transformation * v4{ position.x + width, position.y + height, 0.f, 1.f };
                key.vertex[2].tex_id        = font.atlas.source.id;
                key.vertex[2].tex_coord     = v2{ tex_coord_v.x, tex_coord_u.y };
                key.vertex[2].color         = text.color;
                key.vertex[2].additional    = screen_px_range;

                key.vertex[3].position      = transformation * v4{ position.x, position.y + height, 0.f, 1.f };
                key.vertex[3].tex_id        = font.atlas.source.id;
                key.vertex[3].tex_coord     = tex_coord_u;
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

    void Renderer::on_window_resize(u32 width, u32 height)
    {
        _framebuffer.create_framebuffer(width, height);
        lights.masking.create_framebuffer(width, height);
        lights.raymarching.create_framebuffer(width, height);
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

                const u8 buffer_index = std::to_underlying(key.buffer);
                VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
                for (u64 i = 0; i < vertex_buffer.count(); ++i)
                {
                    vertex_buffer.emplace( std::move(key.vertex[i]) );
                }

                prev_key = std::move(key);
            }

            _flush_buffer(prev_key.buffer, prev_key.program_id);

            _render_queue.clear();
        }
    }

    void Renderer::_flush_framebuffer(const Framebuffer& framebuffer, ShaderID program_id)
    {
        constexpr u8 buffer_index = std::to_underlying(BufferType::Quads);
        VertexBuffer& vertex_buffer = _vertex_arrays[buffer_index].get_vertex_buffer();
        for (Vertex vertex : framebuffer.get_vertices())
        {
            vertex_buffer.emplace( std::move(vertex) );
        }

        _flush_buffer(BufferType::Quads, program_id);
    }

    void Renderer::_flush_buffer(BufferType buffer, ShaderID program_id)
    {
        bind_shader(program_id);

        const u8 buffer_index = std::to_underlying(buffer);
        _vertex_arrays[buffer_index].render();
    }
}
