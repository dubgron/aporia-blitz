#pragma once

#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/drawables/group.hpp"

#include <array>

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

        Color color = Colors::White;

        texture_id tex_id = 0;
        v2 tex_coord{ 0.f };

        f32 additional = 0.f;
    };

    /* Index for VertexArray in Renderer */
    enum class BufferType : u8
    {
        Quads = 0,
        Lines = 1
    };

    struct RenderQueueKey
    {
        BufferType buffer = BufferType::Quads;
        ShaderID program_id = 0;

        std::array<Vertex, 4> vertex;
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

        void bind_to_shader(ShaderID program_id);
        void set_data(const void* data, u64 size);
    };

    struct Framebuffer
    {
        u32 framebuffer_id = 0;
        u32 renderbuffer_id = 0;

        Texture color_buffer;
        std::array<Vertex, 4> vertices;

        void create(i32 width, i32 height);
        void destroy();

        void bind() const;
        void unbind() const;

        // @TODO(dubgron): Make clear more customizable.
        void clear(Color color = Colors::Black);
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
    void add_light_blocker(Group& light_blocker);

    void rendering_init(u32 width, u32 height);
    void rendering_deinit();

    void rendering_begin(const class Window& window, const class Camera& camera);
    void rendering_end(Color color = Colors::Black);

    void draw(const Group& group);
    void draw(const Sprite& sprite);
    void draw(const Rectangle2D& rect);
    void draw(const Line2D& line);
    void draw(const Circle2D& circle);
    void draw(const Text& text);

    void rendering_push_transform(const Transform2D& transform);
    void rendering_pop_transform();

    void resize_framebuffers(u32 width, u32 height);

    /* Predefined shaders */
    extern ShaderID default_shader;
    extern ShaderID font_shader;
    extern ShaderID postprocessing_shader;
    extern ShaderID raymarching_shader;
    extern ShaderID shadowcasting_shader;
}
