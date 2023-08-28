#include "aporia_rendering.hpp"

#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"

#if defined(APORIA_DEBUG)
    #define DEBUG_TEXTURE(texture) \
        ImGui::Begin("DEBUG | Textures"); \
        ImGui::Text("ID: %d, Size: %d x %d", texture.id, texture.width, texture.height); \
        ImGui::Image((void*)(u64)texture.id, ImVec2{ (f32)texture.width, (f32)texture.height }, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f }); \
        ImGui::End();
#else
    #define DEBUG_TEXTURE(...)
#endif

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))

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

    struct IndexBuffer
    {
        u32 id = 0;
        u32 max_size = 0;
        u32 index_count = 0;

        void init(u32 max_objects, u32 count, const u32* indices);
        void deinit();

        void bind() const;
        void unbind() const;
    };

    struct VertexBuffer
    {
        u32 id = 0;
        u32 vertex_count = 0;
        u64 max_size = 0;

        Vertex* data = nullptr;
        u64 count = 0;

        void init(MemoryArena* arena, u32 max_objects, u32 in_count);
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
        String block_name;

        void init(u32 in_max_size, u32 in_binding_index, String in_block_name);
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

    struct RenderQueue
    {
        RenderQueueKey* data = nullptr;
        u64 max_count = 0;
        u64 count = 0;

        void init(MemoryArena* arena, u64 in_count);
        void add(const RenderQueueKey& key);
    };

    static constexpr u64 MAX_RENDERING_QUEUE_SIZE = 100000;
    static constexpr u64 MAX_OBJECTS_PER_DRAW_CALL = 10000;
    static constexpr u64 MAX_LIGHT_SOURCES = 1000;

    static Framebuffer main_framebuffer;

    static RenderQueue rendering_queue;
    static VertexArray vertex_arrays[2];

    // @TODO(dubgron): Move the lighting code to the separate file.
    static bool lighting_enabled = false;
    static Framebuffer masking;
    static Framebuffer raymarching;
    static UniformBuffer lights_uniform_buffer;

    struct LightSources
    {
        LightSource* data = nullptr;
        u64 max_count = 0;
        u64 count = 0;

        void init(MemoryArena* arena, u64 in_count);
    };

    static LightSources light_sources;

    void IndexBuffer::init(u32 max_objects, u32 count, const u32* indices)
    {
        max_size = max_objects * count;
        index_count = count;

        glGenBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);

#if defined(APORIA_EMSCRIPTEN)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_size * sizeof(u32), indices, GL_STATIC_DRAW);
#else
        glNamedBufferData(id, max_size * sizeof(u32), indices, GL_STATIC_DRAW);
#endif
    }

    void IndexBuffer::deinit()
    {
        glDeleteBuffers(1, &id);
    }

    void IndexBuffer::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void IndexBuffer::unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::init(MemoryArena* arena, u32 max_objects, u32 in_count)
    {
        max_size = max_objects * in_count;
        vertex_count = in_count;


#if defined(APORIA_EMSCRIPTEN)
        glGenBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, max_size * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#else
        glCreateBuffers(1, &id);
        glNamedBufferData(id, max_size * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#endif

        data = arena->push<Vertex>(max_size);
        count = 0;
    }

    void VertexBuffer::deinit()
    {
        glDeleteBuffers(1, &id);
    }

    void VertexBuffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void VertexBuffer::unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::add_layout()
    {
        bind();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

#if defined(APORIA_EMSCRIPTEN)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#else
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#endif

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_coord));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, additional));

        unbind();
    }

    void VertexBuffer::flush() const
    {
#if defined(APORIA_EMSCRIPTEN)
        bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), data);
        unbind();
#else
        glNamedBufferSubData(id, 0, count * sizeof(Vertex), data);
#endif
    }

    void VertexArray::init(u32 vertex_count, u32 index_count)
    {
        glGenVertexArrays(1, &id);

        if (vertex_count == 4 && index_count == 6)
        {
            mode = GL_TRIANGLES;
        }
        else if (vertex_count == 2 && index_count == 2)
        {
            mode = GL_LINES;
        }
    }

    void VertexArray::deinit()
    {
        glDeleteVertexArrays(1, &id);
        vertex_buffer.deinit();
        index_buffer.deinit();
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(id);
    }

    void VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void VertexArray::render()
    {
        vertex_buffer.flush();

        this->bind();
        index_buffer.bind();

        const u32 count = index_buffer.index_count * vertex_buffer.count / vertex_buffer.vertex_count;
        glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);

        index_buffer.unbind();
        this->unbind();

        vertex_buffer.count = 0;
    }

    void UniformBuffer::init(u32 in_max_size, u32 in_binding_index, String in_block_name)
    {
        max_size = in_max_size;
        binding_index = in_binding_index;
        block_name = in_block_name;

        glGenBuffers(1, &id);

        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, in_max_size, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, in_binding_index, id);
    }

    void UniformBuffer::deinit()
    {
        glDeleteBuffers(1, &id);
    }

    void UniformBuffer::bind_to_shader(u32 shader_id)
    {
        const u32 buffer_index = glGetUniformBlockIndex(shader_id, *block_name);
        glUniformBlockBinding(shader_id, buffer_index, binding_index);
    }

    void UniformBuffer::set_data(const void* data, u64 size)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Framebuffer::create(i32 width, i32 height)
    {
        APORIA_ASSERT_WITH_MESSAGE(width > 0 && height > 0,
            "Invalid framebuffer dimensions! Width: {}, Height: {}", width, height);

        // Delete potential previous buffers and textures
        destroy();

        // Create new buffers and textures
        color_buffer.width = width;
        color_buffer.height = height;
        color_buffer.channels = 4;

        glGenFramebuffers(1, &framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

        glGenTextures(1, &color_buffer.id);
        glActiveTexture(GL_TEXTURE0 + color_buffer.id);
        glBindTexture(GL_TEXTURE_2D, color_buffer.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer.id, 0);

        glGenRenderbuffers(1, &renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_id);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Setup vertices
        vertex[0].position = v3{ -1.f, -1.f, 0.f };
        vertex[0].tex_coord = v2{ 0.f, 0.f };
        vertex[0].tex_id = color_buffer.id;

        vertex[1].position = v3{ 1.f, -1.f, 0.f };
        vertex[1].tex_coord = v2{ 1.f, 0.f };
        vertex[1].tex_id = color_buffer.id;

        vertex[2].position = v3{ 1.f, 1.f, 0.f };
        vertex[2].tex_coord = v2{ 1.f, 1.f };
        vertex[2].tex_id = color_buffer.id;

        vertex[3].position = v3{ -1.f, 1.f, 0.f };
        vertex[3].tex_coord = v2{ 0.f, 1.f };
        vertex[3].tex_id = color_buffer.id;
    }

    void Framebuffer::destroy()
    {
        glDeleteTextures(1, &color_buffer.id);
        glDeleteRenderbuffers(1, &renderbuffer_id);
        glDeleteFramebuffers(1, &framebuffer_id);
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    }

    void Framebuffer::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::clear(Color color /* = Color::Black */)
    {
        glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void RenderQueue::init(MemoryArena* arena, u64 in_count)
    {
        data = arena->push<RenderQueueKey>(in_count);
        max_count = in_count;
        count = 0;
    }

    void RenderQueue::add(const RenderQueueKey& key)
    {
        APORIA_ASSERT(count < max_count);
        data[count] = key;
        count += 1;
    }

    void LightSources::init(MemoryArena* arena, u64 in_count)
    {
        data = arena->push<LightSource>(in_count);
        max_count = in_count;
        count = 0;
    }

    static void flush_buffer(BufferType buffer, u32 shader_id)
    {
        bind_shader(shader_id);

        const u8 buffer_index = to_underlying(buffer);
        vertex_arrays[buffer_index].render();
    }

    static void flush_rendering_queue()
    {
        if (rendering_queue.count > 0)
        {
            qsort(rendering_queue.data, rendering_queue.count, sizeof(RenderQueueKey),
                [](const void* elem1, const void* elem2) -> i32
                {
                    const RenderQueueKey& key1 = *reinterpret_cast<const RenderQueueKey*>(elem1);
                    const RenderQueueKey& key2 = *reinterpret_cast<const RenderQueueKey*>(elem2);

                    f32 z_diff = key1.vertex[0].position.z - key2.vertex[0].position.z;
                    if (z_diff < FLT_EPSILON && z_diff > -FLT_EPSILON)
                    {
                        i32 buffer_diff = (i32)key1.buffer - (i32)key2.buffer;
                        if (buffer_diff == 0)
                        {
                            i32 shader_diff = key1.shader_id - key2.shader_id;
                            return shader_diff;
                        }
                        return buffer_diff;
                    }
                    return z_diff;
                });

            RenderQueueKey prev_key = rendering_queue.data[0];
            for (u64 idx = 0; idx < rendering_queue.count; ++idx)
            {
                RenderQueueKey& key = rendering_queue.data[idx];

                if (key.shader_id != prev_key.shader_id || key.buffer != prev_key.buffer)
                {
                    flush_buffer(prev_key.buffer, prev_key.shader_id);
                }

                const u8 buffer_index = to_underlying(key.buffer);
                VertexBuffer& vertex_buffer = vertex_arrays[buffer_index].vertex_buffer;

                if (vertex_buffer.count + vertex_buffer.vertex_count > vertex_buffer.max_size)
                {
                    flush_buffer(key.buffer, key.shader_id);
                }

                for (u64 i = 0; i < vertex_buffer.vertex_count; ++i)
                {
                    vertex_buffer.data[vertex_buffer.count] = key.vertex[i];
                    vertex_buffer.count += 1;
                }

                prev_key = key;
            }

            flush_buffer(prev_key.buffer, prev_key.shader_id);

            rendering_queue.count = 0;
        }
    }

    static void flush_framebuffer(const Framebuffer& framebuffer, u32 shader_id)
    {
        constexpr u8 buffer_index = to_underlying(BufferType::Quads);
        VertexBuffer& vertex_buffer = vertex_arrays[buffer_index].vertex_buffer;
        for (const Vertex& vertex : framebuffer.vertex)
        {
            vertex_buffer.data[vertex_buffer.count] = vertex;
            vertex_buffer.count += 1;
        }

        flush_buffer(BufferType::Quads, shader_id);
    }

    bool is_lighting_enabled()
    {
        return lighting_enabled;
    }

    void enable_lighting()
    {
        lighting_enabled = true;

        const i32 width = main_framebuffer.color_buffer.width;
        const i32 height = main_framebuffer.color_buffer.height;

        masking.create(width, height);
        raymarching.create(width, height);

        if (light_sources.data == nullptr)
        {
            light_sources.init(&persistent_arena, MAX_LIGHT_SOURCES);
        }

        lights_uniform_buffer.init(MAX_LIGHT_SOURCES * sizeof(LightSource), 0, "Lights");
        lights_uniform_buffer.bind_to_shader(raymarching_shader);
        lights_uniform_buffer.bind_to_shader(shadowcasting_shader);
    }

    void disable_lighting()
    {
        lighting_enabled = false;

        masking.destroy();
        raymarching.destroy();

        light_sources.count = 0;

        lights_uniform_buffer.deinit();
    }

    void add_light_source(LightSource source)
    {
        APORIA_ASSERT(light_sources.count < light_sources.max_count);
        light_sources.data[light_sources.count] = source;
        light_sources.count += 1;
    }

    void rendering_init(MemoryArena* arena)
    {
        rendering_queue.init(arena, MAX_RENDERING_QUEUE_SIZE);

        // Set VertexArray for opaque Quads
        VertexArray& quads = vertex_arrays[0];
        quads.init(4, 6);
        quads.bind();

        VertexBuffer quads_vbo;
        quads_vbo.init(arena, MAX_OBJECTS_PER_DRAW_CALL, 4);
        quads_vbo.add_layout();
        quads.vertex_buffer = quads_vbo;

        u32* quad_indices = arena->push<u32>(MAX_OBJECTS_PER_DRAW_CALL * 6);
        for (u32 i = 0, offset = 0; i < MAX_OBJECTS_PER_DRAW_CALL * 6; i += 6, offset += 4)
        {
            quad_indices[  i  ] = offset + 0;
            quad_indices[i + 1] = offset + 1;
            quad_indices[i + 2] = offset + 2;

            quad_indices[i + 3] = offset + 2;
            quad_indices[i + 4] = offset + 3;
            quad_indices[i + 5] = offset + 0;
        }

        IndexBuffer quads_ibo;
        quads_ibo.init(MAX_OBJECTS_PER_DRAW_CALL, 6, quad_indices);
        quads.index_buffer = quads_ibo;

        quads.unbind();

        // Set VertexArray for Lines
        VertexArray& lines = vertex_arrays[1];
        lines.init(2, 2);
        lines.bind();

        VertexBuffer lines_vbo;
        lines_vbo.init(arena, MAX_OBJECTS_PER_DRAW_CALL, 2);
        lines_vbo.add_layout();
        lines.vertex_buffer = lines_vbo;

        u32* line_indices = arena->push<u32>(MAX_OBJECTS_PER_DRAW_CALL * 2);
        for (u32 i = 0; i < MAX_OBJECTS_PER_DRAW_CALL * 2; ++i)
        {
            line_indices[i] = i;
        }

        IndexBuffer lines_ibo;
        lines_ibo.init(MAX_OBJECTS_PER_DRAW_CALL, 2, line_indices);
        lines.index_buffer = lines_ibo;

        lines.unbind();

        // Setup Framebuffer
        main_framebuffer.create(window_config.width, window_config.height);

        // Initialize texture sampler
        std::array<i32, OPENGL_MAX_TEXTURE_UNITS> sampler{};
        std::iota(sampler.begin(), sampler.end(), 0);

        // Setup default shaders
        default_shader = create_shader("content/shaders/default.glsl");
        circle_shader = create_shader("content/shaders/circle.glsl");
        line_shader = create_shader("content/shaders/line.glsl");
        font_shader = create_shader("content/shaders/font.glsl");

        bind_shader(default_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        bind_shader(font_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        // Setup post-processing shaders
        postprocessing_shader = create_shader("content/shaders/postprocessing.glsl");

        bind_shader(postprocessing_shader);
        shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

        // Setup lighting shaders
        raymarching_shader = create_shader("content/shaders/raymarching.glsl");
        shadowcasting_shader = create_shader("content/shaders/shadowcasting.glsl");

        // Setup editor grid shaders
        editor_grid_shader = create_shader("content/shaders/editor_grid.glsl");

        unbind_shader();
    }

    void rendering_deinit()
    {
        for (u64 idx = 0; idx < ARRAY_COUNT(vertex_arrays); ++idx)
        {
            vertex_arrays[idx].deinit();
        }

        main_framebuffer.destroy();
        if (lighting_enabled)
        {
            disable_lighting();
        }
        remove_all_shaders();
    }

    void rendering_begin()
    {
        light_sources.count = 0;
    }

    void rendering_end()
    {
        main_framebuffer.bind();
        main_framebuffer.clear(camera_config.background_color);

        const m4& view_projection_matrix = active_camera->calculate_view_projection_matrix();
        const f32 camera_zoom = 1.f / active_camera->projection.zoom;

        bind_shader(default_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(line_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(circle_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(font_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);
        shader_set_float("u_camera_zoom", camera_zoom);

        v2 window_size = v2{ active_window->get_size() };

        bind_shader(raymarching_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);
        shader_set_int("u_masking", masking.color_buffer.id);
        shader_set_float("u_camera_zoom", camera_zoom);
        shader_set_float2("u_window_size", window_size);

        bind_shader(shadowcasting_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);
        shader_set_int("u_raymarching", raymarching.color_buffer.id);
        shader_set_float("u_camera_zoom", camera_zoom);
        shader_set_float2("u_window_size", window_size);

        if (editor_config.display_editor_grid)
        {
            bind_shader(editor_grid_shader);
            shader_set_mat4("u_vp_matrix", view_projection_matrix);

            VertexArray& quad_vertex_array = vertex_arrays[(u64)BufferType::Quads];

            const u64 idx = quad_vertex_array.vertex_buffer.count;
            quad_vertex_array.vertex_buffer.data[idx + 0] = Vertex{ v3{ -1.f, -1.f, 0.f } };
            quad_vertex_array.vertex_buffer.data[idx + 1] = Vertex{ v3{  1.f, -1.f, 0.f } };
            quad_vertex_array.vertex_buffer.data[idx + 2] = Vertex{ v3{  1.f,  1.f, 0.f } };
            quad_vertex_array.vertex_buffer.data[idx + 3] = Vertex{ v3{ -1.f,  1.f, 0.f } };
            quad_vertex_array.vertex_buffer.count += 4;

            bind_shader(editor_grid_shader);
            quad_vertex_array.render();
        }

        flush_rendering_queue();
        main_framebuffer.unbind();

        if (lighting_enabled)
        {
            lights_uniform_buffer.set_data(light_sources.data, light_sources.count * sizeof(LightSource));

            bind_shader(raymarching_shader);
            shader_set_uint("u_num_lights", light_sources.count);

            bind_shader(shadowcasting_shader);
            shader_set_uint("u_num_lights", light_sources.count);

            masking.bind();
            masking.clear(Color::Transparent);

            for (u64 idx = 0; idx < world.entity_count; ++idx)
            {
                const Entity& entity = world.entity_array[idx];
                if (is_flag_set(entity, EntityFlag_Visible) && is_flag_set(entity, EntityFlag_BlockingLight))
                {
                    draw_entity(entity);
                }
            }

            flush_rendering_queue();
            masking.unbind();

            DEBUG_TEXTURE(masking.color_buffer);

            raymarching.bind();
            raymarching.clear(Color::Black);
            flush_framebuffer(masking, raymarching_shader);
            raymarching.unbind();

            DEBUG_TEXTURE(raymarching.color_buffer);

            main_framebuffer.bind();
            flush_framebuffer(raymarching, shadowcasting_shader);
            main_framebuffer.unbind();
        }

        DEBUG_TEXTURE(main_framebuffer.color_buffer);

        flush_framebuffer(main_framebuffer, postprocessing_shader);

        unbind_shader();
    }

    void draw_entity(const Entity& entity)
    {
        const f32 sin = std::sin(entity.rotation);
        const f32 cos = std::cos(entity.rotation);

        const v3 right_offset       = v3{ cos, sin, 0.f } * entity.width * entity.scale.x;
        const v3 up_offset          = v3{ -sin, cos, 0.f } * entity.height * entity.scale.y;

        const v3 offset_from_center = right_offset * entity.center_of_rotation.x + up_offset * entity.center_of_rotation.y;
        const v3 base_offset        = v3{ entity.position, entity.z } - offset_from_center;

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.shader_id               = entity.shader_id;

        key.vertex[0].position      = base_offset;
        key.vertex[0].color         = entity.color;

        key.vertex[1].position      = base_offset + right_offset;
        key.vertex[1].color         = entity.color;

        key.vertex[2].position      = base_offset + right_offset + up_offset;
        key.vertex[2].color         = entity.color;

        key.vertex[3].position      = base_offset + up_offset;
        key.vertex[3].color         = entity.color;

        if (entity.texture)
        {
            // @NOTE(dubgron): Disabled the half pixel offset as it is only useful when dealing with rotated tiles.
            // const v2 half_pixel_offset  = 0.5f / v2{ entity.texture->source.width, entity.texture->source.height };
            // const v2 tex_coord_u        = entity.texture->u + half_pixel_offset;
            // const v2 tex_coord_v        = entity.texture->v - half_pixel_offset;
            const v2 tex_coord_u        = entity.texture->u;
            const v2 tex_coord_v        = entity.texture->v;

            key.vertex[0].tex_coord     = v2{ tex_coord_u.x, tex_coord_v.y };
            key.vertex[0].tex_id        = entity.texture->source.id;

            key.vertex[1].tex_coord     = tex_coord_v;
            key.vertex[1].tex_id        = entity.texture->source.id;

            key.vertex[2].tex_coord     = v2{ tex_coord_v.x, tex_coord_u.y };
            key.vertex[2].tex_id        = entity.texture->source.id;

            key.vertex[3].tex_coord     = tex_coord_u;
            key.vertex[3].tex_id        = entity.texture->source.id;
        }

        rendering_queue.add(key);
    }

    void draw_rectangle(v2 position, f32 width, f32 height, Color color /* = Color::White */, u32 shader_id /* = default_shader */)
    {
        const v3 base_offset    = v3{ position, 0.f };
        const v3 right_offset   = v3{ width, 0.f, 0.f };
        const v3 up_offset      = v3{ 0.f, height, 0.f };

        RenderQueueKey key;
        key.buffer              = BufferType::Quads;
        key.shader_id           = shader_id;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = color;
        key.vertex[0].tex_coord = v2{ 0.f, 0.f };

        key.vertex[1].position  = base_offset + right_offset;
        key.vertex[1].color     = color;
        key.vertex[1].tex_coord = v2{ 1.f, 0.f };

        key.vertex[2].position  = base_offset + right_offset + up_offset;
        key.vertex[2].color     = color;
        key.vertex[2].tex_coord = v2{ 1.f, 1.f };

        key.vertex[3].position  = base_offset + up_offset;
        key.vertex[3].color     = color;
        key.vertex[3].tex_coord = v2{ 0.f, 1.f };

        rendering_queue.add(key);
    }

    void draw_line(v2 begin, v2 end, f32 thickness /* = 1.f */, Color color /* = Color::White */, u32 shader_id /* = line_shader */)
    {
        const v2 direction = glm::normalize(end - begin);
        const v2 normal = v2{ -direction.y, direction.x };

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.shader_id               = shader_id;

        key.vertex[0].position      = v3{ begin, 0.f };
        key.vertex[0].tex_coord     = normal;
        key.vertex[0].color         = color;
        key.vertex[0].additional    = thickness;

        key.vertex[1].position      = v3{ begin, 0.f };
        key.vertex[1].tex_coord     = -normal;
        key.vertex[1].color         = color;
        key.vertex[1].additional    = thickness;

        key.vertex[2].position      = v3{ end, 0.f };
        key.vertex[2].tex_coord     = -normal;
        key.vertex[2].color         = color;
        key.vertex[2].additional    = thickness;

        key.vertex[3].position      = v3{ end, 0.f };
        key.vertex[3].tex_coord     = normal;
        key.vertex[3].color         = color;
        key.vertex[3].additional    = thickness;

        rendering_queue.add(key);
    }

    void draw_circle(v2 position, f32 radius, Color color /* = Color::White */, u32 shader_id /* = circle_shader */)
    {
        const v3 base_offset          = v3{ position, 0.f };
        const v3 right_half_offset    = v3{ -radius, 0.f, 0.f };
        const v3 up_half_offset       = v3{ 0.f, radius, 0.f };

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.shader_id               = shader_id;

        key.vertex[0].position      = base_offset - right_half_offset - up_half_offset;
        key.vertex[0].color         = color;
        key.vertex[0].tex_coord     = v2{ -1.f, -1.f };

        key.vertex[1].position      = base_offset + right_half_offset - up_half_offset;
        key.vertex[1].color         = color;
        key.vertex[1].tex_coord     = v2{ 1.f, -1.f };

        key.vertex[2].position      = base_offset + right_half_offset + up_half_offset;
        key.vertex[2].color         = color;
        key.vertex[2].tex_coord     = v2{ 1.f, 1.f };

        key.vertex[3].position      = base_offset - right_half_offset + up_half_offset;
        key.vertex[3].color         = color;
        key.vertex[3].tex_coord     = v2{ -1.f, 1.f };

        rendering_queue.add(key);
    }

    // @TODO(dubgron): The current implementation of aligning text is shitty and hard to read, so it needs refactor.
    void draw_text(const Text& text)
    {
        if (text.caption.length == 0)
        {
            return;
        }

        APORIA_ASSERT(text.font);
        const Font& font = *text.font;

        const v2 texture_size           = v2{ font.atlas.source.width, font.atlas.source.height };

        // Adjust text scaling by the predefined atlas font size
        const f32 effective_font_size   = text.font_size / font.atlas.font_size;
        const f32 screen_px_range       = font.atlas.distance_range * effective_font_size;

        const f32 sin = std::sin(text.rotation);
        const f32 cos = std::cos(text.rotation);

        u64 line_count = 1;
        for (u64 idx = 0; idx < text.caption.length; ++idx)
        {
            if (text.caption.data[idx] == '\n')
            {
                line_count += 1;
            }
        }

        ScratchArena temp = create_scratch_arena(&persistent_arena);

        f32* line_alignments = temp.arena->push_zero<f32>(line_count);
        f32 max_line_alignment = 0.f;

        // Calculate the alignment of every text line
        {
            u64 current_line = 0;
            for (u64 idx = 0; idx < text.caption.length; ++idx)
            {
                const u8 character = text.caption.data[idx];
                if (idx > 0)
                {
                    const u8 prev_character = text.caption.data[idx - 1];

                    const std::pair<u8, u8> key = std::make_pair(prev_character, character);
                    if (font.kerning.contains(key))
                    {
                        line_alignments[current_line] += font.kerning.at(key);
                    }

                    if (font.glyphs.contains(prev_character))
                    {
                        line_alignments[current_line] += font.glyphs.at(prev_character).advance;
                    }
                }

                if (character == ' ')
                {
                    line_alignments[current_line] += font.metrics.em_size / 4.f;
                }
                else if (character == '\t')
                {
                    line_alignments[current_line] += font.metrics.em_size * 2.f;
                }
                else if (character == '\n')
                {
                    max_line_alignment = max(max_line_alignment, line_alignments[current_line]);
                    current_line += 1;
                }
            }

            const u8 last_character = text.caption.data[text.caption.length - 1];
            if (font.glyphs.contains(last_character))
            {
                line_alignments[line_count - 1] += font.glyphs.at(last_character).advance;
            }

            max_line_alignment = max(max_line_alignment, line_alignments[line_count - 1]);

            static constexpr f32 align_blend[] = { 0.f, 0.5f, 1.f };
            const u64 alignment_id = static_cast<u64>(text.alignment);

            for (u64 idx = 0; idx < line_count; ++idx)
            {
                line_alignments[idx] = (max_line_alignment - line_alignments[idx]) * align_blend[alignment_id];
            }
        }

        // @TOOD(dubgron): Fix this. Right now we don't load x-height from font, so we have to approximate it.
        const f32 x_height = font.metrics.line_height * 0.65f;
        const f32 total_text_height = (line_count - 1) * font.metrics.line_height + x_height;
        const v2 center_offset = v2{ max_line_alignment, total_text_height } * text.center_of_rotation;

        u64 current_line = 0;
        v2 advance{ 0.f, total_text_height - x_height };
        for (u64 idx = 0; idx < text.caption.length; ++idx)
        {
            const u8 character = text.caption.data[idx];

            if (idx > 0)
            {
                const u8 prev_character = text.caption.data[idx - 1];

                const std::pair<u8, u8> key = std::make_pair(prev_character, character);
                if (font.kerning.contains(key))
                {
                    advance.x += font.kerning.at(key);
                }

                if (font.glyphs.contains(prev_character))
                {
                    advance.x += font.glyphs.at(prev_character).advance;
                }
            }

            if (character == ' ')
            {
                advance.x += font.metrics.em_size / 4.f;
            }
            else if (character == '\t')
            {
                advance.x += font.metrics.em_size * 2.f;
            }
            else if (character == '\n')
            {
                advance.x = 0.f;
                advance.y -= font.metrics.line_height;

                current_line += 1;
            }
            else if (font.glyphs.contains(character))
            {
                const Glyph& glyph = font.glyphs.at(character);

                const GlyphBounds& atlas_bounds = glyph.atlas_bounds;
                const GlyphBounds& plane_bounds = glyph.plane_bounds;

                const v2 tex_coord_u    = v2{ atlas_bounds.left, atlas_bounds.top } / texture_size;
                const v2 tex_coord_v    = v2{ atlas_bounds.right, atlas_bounds.bottom } / texture_size;

                const f32 width         = (atlas_bounds.right - atlas_bounds.left) * effective_font_size;
                const f32 height        = (atlas_bounds.bottom - atlas_bounds.top) * effective_font_size;

                // @NOTE(dubgron): We flpi the sign of plane_bounds.bottom because
                // the plane_bounds lives in a space where the y-axis goes downwards.
                const v2 plane_offset   = v2{ plane_bounds.left, -plane_bounds.bottom };
                const v2 align_offset   = v2{ line_alignments[current_line], 0.f };
                const v2 line_offset    = advance + plane_offset + align_offset - center_offset;

                const f32 rotated_x     = cos * line_offset.x - sin * line_offset.y;
                const f32 rotated_y     = sin * line_offset.x + cos * line_offset.y;

                const v2 base_offset    = text.position + v2{ rotated_x, rotated_y } * text.font_size;
                const v2 right_offset   = v2{ cos , sin } * width;
                const v2 up_offset      = v2{ -sin, cos } * height;

                RenderQueueKey key;
                key.buffer                  = BufferType::Quads;
                key.shader_id               = text.shader_id;

                key.vertex[0].position      = v3{ base_offset, 0.f };
                key.vertex[0].tex_id        = font.atlas.source.id;
                key.vertex[0].tex_coord     = v2{ tex_coord_u.x, tex_coord_v.y };
                key.vertex[0].color         = text.color;
                key.vertex[0].additional    = screen_px_range;

                key.vertex[1].position      = v3{ base_offset + right_offset, 0.f };
                key.vertex[1].tex_id        = font.atlas.source.id;
                key.vertex[1].tex_coord     = tex_coord_v;
                key.vertex[1].color         = text.color;
                key.vertex[1].additional    = screen_px_range;

                key.vertex[2].position      = v3{ base_offset + right_offset + up_offset, 0.f };
                key.vertex[2].tex_id        = font.atlas.source.id;
                key.vertex[2].tex_coord     = v2{ tex_coord_v.x, tex_coord_u.y };
                key.vertex[2].color         = text.color;
                key.vertex[2].additional    = screen_px_range;

                key.vertex[3].position      = v3{ base_offset + up_offset, 0.f };
                key.vertex[3].tex_id        = font.atlas.source.id;
                key.vertex[3].tex_coord     = tex_coord_u;
                key.vertex[3].color         = text.color;
                key.vertex[3].additional    = screen_px_range;

                rendering_queue.add(key);
            }
        }

        rollback_scratch_arena(temp);
    }

    void resize_framebuffers(u32 width, u32 height)
    {
        main_framebuffer.create(width, height);
        if (lighting_enabled)
        {
            masking.create(width, height);
            raymarching.create(width, height);
        }
    }
}
