#pragma once

#include "aporia_entity.hpp"
#include "aporia_fonts.hpp"
#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
#if defined(APORIA_EMSCRIPTEN)
    using texture_id = f32;
#else
    using texture_id = u32;
#endif

    struct Vertex
    {
        v3 position{ 0.f };

        Color color = Color::White;

        texture_id tex_id = 0;
        v2 tex_coord{ 0.f };

        f32 additional = 0.f;
    };

    // Index for VertexArray in Renderer
    enum class BufferType : u8
    {
        Quads = 0,
        Lines = 1
    };

    struct RenderQueueKey
    {
        BufferType buffer = BufferType::Quads;
        u32 shader_id = 0;

        Vertex vertex[4];
    };

    bool operator<(const RenderQueueKey& key1, const RenderQueueKey& key2) noexcept;

    struct IndexBuffer
    {
        u32 id = 0;
        u32 max_size = 0;
        u32 index_count = 0;

        void init(u32 max_objects, u32 count, const std::vector<u32>& indices);
        void deinit();

        void bind() const;
        void unbind() const;
    };

    struct VertexBuffer
    {
        u32 id = 0;
        u32 max_size = 0;
        u32 vertex_count = 0;

        std::vector<Vertex> data;

        void init(u32 max_objects, u32 count);
        void deinit();

        void bind() const;
        void unbind() const;

        void add_layout();

        void flush() const;
    };

    struct VertexArray
    {
        u32 id = 0;
        u32 mode = 0;

        VertexBuffer vertex_buffer;
        IndexBuffer index_buffer;

        void init(u32 vertex_count, u32 index_count);
        void deinit();

        void bind() const;
        void unbind() const;

        void render();
    };

    struct UniformBuffer
    {
        u32 id = 0;
        u32 max_size = 0;
        u32 binding_index = 0;
        std::string block_name;

        void init(u32 in_max_size, u32 in_binding_index, std::string_view in_block_name);
        void deinit();

        void bind_to_shader(u32 shader_id);
        void set_data(const void* data, u64 size);
    };

    struct Framebuffer
    {
        u32 framebuffer_id = 0;
        u32 renderbuffer_id = 0;

        Texture color_buffer;
        Vertex vertex[4];

        void create(i32 width, i32 height);
        void destroy();

        void bind() const;
        void unbind() const;

        // @TODO(dubgron): Make clear more customizable.
        void clear(Color color = Color::Black);
    };

    struct LightSource
    {
        v2 origin{ 0.f };
        f32 range = 1.f;
        f32 falloff = 2.f;

        v3 color{ 1.f };
        f32 intensity = 1.f;
    };

    bool is_lighting_enabled();
    void enable_lighting();
    void disable_lighting();

    void add_light_source(LightSource light_source);
    u32 light_source_count();

    void rendering_init();
    void rendering_deinit();

    void rendering_begin();
    void rendering_end();

    void draw_entity(const Entity& entity);
    void draw_rectangle(v2 position, f32 width, f32 height, Color color, u32 shader_id = default_shader);
    void draw_line(v2 begin, v2 end, f32 thickness, Color color, u32 shader_id = line_shader);
    void draw_circle(v2 position, f32 radius, Color color, u32 shader_id = circle_shader);
    void draw_text(const Text& text);

    void resize_framebuffers(u32 width, u32 height);
}
