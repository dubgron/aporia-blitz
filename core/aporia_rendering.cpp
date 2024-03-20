#include "aporia_rendering.hpp"

#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"

// @NOTE(dubgron): We need to set the uvs to (0, 1) and (1, 0), because
// ImGui expects them to be the coordinates of the top-left corner and
// the bottom-right corner of the texture (respectively). The default
// uvs of (0, 0) and (1, 1) are fine for the screen coordinates (where
// the point (0, 0) is in the top-left corner, and the y axis grows
// downwards), but an OpenGL texture has its origin at the bottom-left
// corner of the screen and the y axis grows upwards.
#if defined(APORIA_DEBUG)
    #define DEBUG_TEXTURE(texture) \
        ImGui::Begin("DEBUG | Textures"); \
        ImGui::Text("ID: %d, Size: %d x %d", (texture).id, (texture).width, (texture).height); \
        ImGui::Image((void*)(u64)(texture).id, ImVec2{ (f32)(texture).width, (f32)(texture).height }, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f }); \
        ImGui::End();
#else
    #define DEBUG_TEXTURE(...)
#endif

namespace Aporia
{
    static constexpr u64 MAX_RENDERING_QUEUE_SIZE = 100000;
    static constexpr u64 MAX_OBJECTS_PER_DRAW_CALL = 10000;

    // @NOTE(dubgron); It maps texture units to texture ids.
    static u32 textures_used_in_draw_call[OPENGL_MAX_TEXTURE_UNITS] = { 0 };
    static u32 first_unused_texture_unit = 0;

    static constexpr u32 INVALID_TEXTURE_UNIT = -1;

    u32 find_or_assign_texture_unit(u32 texture_id)
    {
        for (u64 texture_unit = 0; texture_unit < first_unused_texture_unit; ++texture_unit)
        {
            if (textures_used_in_draw_call[texture_unit] == texture_id)
            {
#if defined(APORIA_EMSCRIPTEN)
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, texture_id);
#else
                glBindTextureUnit(texture_unit, texture_id);
#endif
                return texture_unit;
            }
        }

        if (first_unused_texture_unit < ARRAY_COUNT(textures_used_in_draw_call))
        {
            u32 texture_unit = first_unused_texture_unit;

            textures_used_in_draw_call[texture_unit] = texture_id;
            first_unused_texture_unit += 1;

#if defined(APORIA_EMSCRIPTEN)
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, texture_id);
#else
            glBindTextureUnit(texture_unit, texture_id);
#endif
            return texture_unit;
        }

        return INVALID_TEXTURE_UNIT;
    }

#if defined(APORIA_EMSCRIPTEN)
    using texture_unit = f32;
#else
    using texture_unit = u32;
#endif

    struct Vertex
    {
        v3 position{ 0.f };

        Color color = Color::White;

        texture_unit tex_unit = 0;
        v2 tex_coord{ 0.f };

        f32 additional = 0.f;
    };

    struct IndexBuffer
    {
        u32 id = 0;
        u32 max_count = 0;
        u32 index_per_object = 0;
    };

    static IndexBuffer indexbuffer_create(u32 max_count, u32 index_per_object, u32* indices)
    {
        IndexBuffer result;
        result.max_count = max_count;
        result.index_per_object = index_per_object;

        i64 size = result.max_count * sizeof(u32);

#if defined(APORIA_EMSCRIPTEN)
        glGenBuffers(1, &result.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
#else
        glCreateBuffers(1, &result.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.id);
        glNamedBufferData(result.id, size, indices, GL_STATIC_DRAW);
#endif

        return result;
    }

    static void indexbuffer_destroy(IndexBuffer* index_buffer)
    {
        glDeleteBuffers(1, &index_buffer->id);
    }

    static void indexbuffer_bind(IndexBuffer* index_buffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer->id);
    }

    static void indexbuffer_unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    struct VertexBuffer
    {
        u32 id = 0;
        u32 max_count = 0;
        u32 vertex_per_object = 0;

        Vertex* data = nullptr;
        u64 count = 0;
    };

    static VertexBuffer vertexbuffer_create(MemoryArena* arena, u32 max_count, u32 vertex_per_object)
    {
        VertexBuffer result;
        result.max_count = max_count;
        result.vertex_per_object = vertex_per_object;

        result.data = arena_push_uninitialized<Vertex>(arena, result.max_count);
        result.count = 0;

        i64 size = result.max_count * sizeof(Vertex);

#if defined(APORIA_EMSCRIPTEN)
        glGenBuffers(1, &result.id);
        glBindBuffer(GL_ARRAY_BUFFER, result.id);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#else
        glCreateBuffers(1, &result.id);
        glBindBuffer(GL_ARRAY_BUFFER, result.id);
        glNamedBufferData(result.id, size, nullptr, GL_DYNAMIC_DRAW);
#endif

        return result;
    }

    static void vertexbuffer_destroy(VertexBuffer* vertex_buffer)
    {
        glDeleteBuffers(1, &vertex_buffer->id);
    }

    static void vertexbuffer_bind(VertexBuffer* vertex_buffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
    }

    static void vertexbuffer_unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    static void vertexbuffer_add_layout(VertexBuffer* vertex_buffer)
    {
        vertexbuffer_bind(vertex_buffer);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

#if defined(APORIA_EMSCRIPTEN)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_unit));
#else
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, tex_unit));
#endif

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, additional));

        vertexbuffer_unbind();
    }

    static void vertexbuffer_flush(VertexBuffer* vertex_buffer)
    {
#if defined(APORIA_EMSCRIPTEN)
        vertexbuffer_bind(vertex_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer->count * sizeof(Vertex), vertex_buffer->data);
        vertexbuffer_unbind();
#else
        glNamedBufferSubData(vertex_buffer->id, 0, vertex_buffer->count * sizeof(Vertex), vertex_buffer->data);
#endif
    }

    struct VertexArray
    {
        u32 id = 0;
        u32 mode = 0;

        VertexBuffer vertex_buffer;
        IndexBuffer index_buffer;
    };

    static VertexArray vertexarray_create(u32 vertex_stride, u32 index_stride)
    {
        VertexArray result;
        glGenVertexArrays(1, &result.id);

        if (vertex_stride == 4 && index_stride == 6)
        {
            result.mode = GL_TRIANGLES;
        }
        else if (vertex_stride == 2 && index_stride == 2)
        {
            result.mode = GL_LINES;
        }

        return result;
    }

    static void vertexarray_destroy(VertexArray* vertex_array)
    {
        glDeleteVertexArrays(1, &vertex_array->id);
        vertexbuffer_destroy(&vertex_array->vertex_buffer);
        indexbuffer_destroy(&vertex_array->index_buffer);
    }

    static void vertexarray_bind(VertexArray* vertex_array)
    {
        glBindVertexArray(vertex_array->id);
    }

    static void vertexarray_unbind()
    {
        glBindVertexArray(0);
    }

    static void vertexarray_render(VertexArray* vertex_array)
    {
        vertexbuffer_flush(&vertex_array->vertex_buffer);

        vertexarray_bind(vertex_array);
        indexbuffer_bind(&vertex_array->index_buffer);

        u32 index_count = vertex_array->index_buffer.index_per_object * vertex_array->vertex_buffer.count / vertex_array->vertex_buffer.vertex_per_object;
        glDrawElements(vertex_array->mode, index_count, GL_UNSIGNED_INT, nullptr);

        indexbuffer_unbind();
        vertexarray_unbind();

        // @NOTE(dubgron): Here we could also memset vertex_buffer.data and textures_used_in_draw_call to zero.
        vertex_array->vertex_buffer.count = 0;
        first_unused_texture_unit = 0;
    }

    struct UniformBuffer
    {
        u32 id = 0;
        u32 max_size = 0;
        u32 binding_index = 0;
        String block_name;
    };

    static UniformBuffer uniformbuffer_create(u32 max_size, u32 binding_index, String block_name)
    {
        UniformBuffer result;
        result.max_size = max_size;
        result.binding_index = binding_index;
        result.block_name = block_name;


#if defined(APORIA_EMSCRIPTEN)
        glGenBuffers(1, &result.id);
        glBindBuffer(GL_UNIFORM_BUFFER, result.id);
        glBufferData(GL_UNIFORM_BUFFER, result.max_size, nullptr, GL_STATIC_DRAW);
#else
        glCreateBuffers(1, &result.id);
        glNamedBufferData(result.id, result.max_size, nullptr, GL_STATIC_DRAW);
#endif

        glBindBufferBase(GL_UNIFORM_BUFFER, result.binding_index, result.id);

        return result;
    }

    static void uniformbuffer_destroy(UniformBuffer* uniform_buffer)
    {
        glDeleteBuffers(1, &uniform_buffer->id);
    }

    static void uniformbuffer_bind_to_shader(UniformBuffer* uniform_buffer, u32 shader_id)
    {
        u32 buffer_index = glGetUniformBlockIndex(shader_id, *uniform_buffer->block_name);
        glUniformBlockBinding(shader_id, buffer_index, uniform_buffer->binding_index);
    }

    static void uniformbuffer_set_data(UniformBuffer* uniform_buffer, void* data, u64 size)
    {
#if defined(APORIA_EMSCRIPTEN)
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer->id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
#else
        glNamedBufferSubData(uniform_buffer->id, 0, size, data);
#endif
    }

    // Index for VertexArray in Renderer
    enum class BufferType : u8
    {
        Quads = 0,
        Lines = 1,
    };

    struct RenderQueueKey
    {
        BufferType buffer = BufferType::Quads;
        u32 shader_id = 0;
        u32 texture_id = 0;

        Vertex vertex[4];
    };

    struct RenderQueue
    {
        RenderQueueKey* data = nullptr;
        u64 max_count = 0;
        u64 count = 0;
    };

    static RenderQueue rendering_queue;
    static VertexArray vertex_arrays[2];

    static VertexArray* get_vao_from_buffer(BufferType buffer_type)
    {
        return &vertex_arrays[(u64)buffer_type];
    }

    static RenderQueue renderqueue_create(MemoryArena* arena, u64 in_count)
    {
        RenderQueue result;
        result.data = arena_push_uninitialized<RenderQueueKey>(arena, in_count);
        result.max_count = in_count;
        result.count = 0;
        return result;
    }

    static void renderqueue_add(RenderQueue* render_queue, const RenderQueueKey& key)
    {
        APORIA_ASSERT(render_queue->count < render_queue->max_count);
        render_queue->data[render_queue->count] = key;
        render_queue->count += 1;
    }

    static void renderqueue_flush(RenderQueue* render_queue)
    {
        if (render_queue->count == 0)
        {
            return;
        }

        qsort(render_queue->data, render_queue->count, sizeof(RenderQueueKey),
            [](const void* elem1, const void* elem2) -> i32
            {
                const RenderQueueKey& key1 = *(RenderQueueKey*)elem1;
                const RenderQueueKey& key2 = *(RenderQueueKey*)elem2;

                f32 z_diff = key1.vertex[0].position.z - key2.vertex[0].position.z;
                if (z_diff < FLT_EPSILON && z_diff > -FLT_EPSILON)
                {
                    i32 buffer_diff = (i32)key1.buffer - (i32)key2.buffer;
                    if (buffer_diff == 0)
                    {
                        i32 shader_diff = key1.shader_id - key2.shader_id;
                        if (shader_diff == 0)
                        {
                            uintptr_t ptr_diff = PTR_TO_INT(elem1) - PTR_TO_INT(elem2);
                            return ptr_diff;
                        }
                        return shader_diff;
                    }
                    return buffer_diff;
                }
                return z_diff;
            });

        RenderQueueKey* prev_key = &render_queue->data[0];
        for (u64 idx = 0; idx < render_queue->count; ++idx)
        {
            RenderQueueKey* key = &render_queue->data[idx];

            if (key->shader_id != prev_key->shader_id || key->buffer != prev_key->buffer)
            {
                bind_shader(prev_key->shader_id);
                vertexarray_render(get_vao_from_buffer(prev_key->buffer));
            }

            VertexArray* vertex_array = get_vao_from_buffer(key->buffer);
            VertexBuffer* vertex_buffer = &vertex_array->vertex_buffer;

            u32 texture_unit = find_or_assign_texture_unit(key->texture_id);

            bool no_available_texture_unit = (texture_unit == INVALID_TEXTURE_UNIT);
            bool vertex_buffer_overflow = (vertex_buffer->count + vertex_buffer->vertex_per_object > vertex_buffer->max_count);

            if (no_available_texture_unit || vertex_buffer_overflow)
            {
                bind_shader(key->shader_id);
                vertexarray_render(vertex_array);

                texture_unit = find_or_assign_texture_unit(key->texture_id);
            }

            for (u64 i = 0; i < vertex_buffer->vertex_per_object; ++i)
            {
                key->vertex[i].tex_unit = texture_unit;

                vertex_buffer->data[vertex_buffer->count] = key->vertex[i];
                vertex_buffer->count += 1;
            }

            prev_key = key;
        }

        bind_shader(prev_key->shader_id);
        vertexarray_render(get_vao_from_buffer(prev_key->buffer));

        render_queue->count = 0;
    }

    struct Framebuffer
    {
        u32 framebuffer_id = 0;
        u32 renderbuffer_id = 0;

        Texture color_buffer;
        Vertex vertex[4];
    };

    static Framebuffer main_framebuffer;
    static Framebuffer temp_framebuffer;

    static Framebuffer framebuffer_create(i32 width, i32 height)
    {
        APORIA_ASSERT_WITH_MESSAGE(width > 0 && height > 0,
            "Invalid framebuffer dimensions! Width: %, Height: %", width, height);

        Framebuffer result;

        // Create new buffers and textures
        result.color_buffer.width = width;
        result.color_buffer.height = height;
        result.color_buffer.channels = 4;

        glGenFramebuffers(1, &result.framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, result.framebuffer_id);

        glGenTextures(1, &result.color_buffer.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, result.color_buffer.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.color_buffer.id, 0);

        glGenRenderbuffers(1, &result.renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, result.renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.renderbuffer_id);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Setup vertices
        result.vertex[0].position   = v3{ -1.f, -1.f, 0.f };
        result.vertex[0].tex_coord  = v2{ 0.f, 0.f };

        result.vertex[1].position   = v3{ 1.f, -1.f, 0.f };
        result.vertex[1].tex_coord  = v2{ 1.f, 0.f };

        result.vertex[2].position   = v3{ 1.f, 1.f, 0.f };
        result.vertex[2].tex_coord  = v2{ 1.f, 1.f };

        result.vertex[3].position   = v3{ -1.f, 1.f, 0.f };
        result.vertex[3].tex_coord  = v2{ 0.f, 1.f };

        return result;
    }

    static void framebuffer_destroy(Framebuffer* framebuffer)
    {
        glDeleteTextures(1, &framebuffer->color_buffer.id);
        glDeleteRenderbuffers(1, &framebuffer->renderbuffer_id);
        glDeleteFramebuffers(1, &framebuffer->framebuffer_id);
    }

    static void framebuffer_resize(Framebuffer* framebuffer, i32 width, i32 height)
    {
        framebuffer_destroy(framebuffer);
        *framebuffer = framebuffer_create(width, height);
    }

    static void framebuffer_bind(const Framebuffer& framebuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer_id);
    }

    static void framebuffer_unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static void framebuffer_clear(Color color /* = Color::Black */)
    {
        glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static void framebuffer_flush(const Framebuffer& framebuffer, u32 shader_id)
    {
        VertexArray* quads = get_vao_from_buffer(BufferType::Quads);

        for (const Vertex& vertex : framebuffer.vertex)
        {
            quads->vertex_buffer.data[quads->vertex_buffer.count] = vertex;
            quads->vertex_buffer.count += 1;
        }

        bind_shader(shader_id);
        vertexarray_render(quads);
    }

    struct LightSourceArray
    {
        LightSource* data = nullptr;
        u64 max_count = 0;
        u64 count = 0;
    };

    static constexpr u64 MAX_LIGHT_SOURCES = 1000;

    // @TODO(dubgron): Move the lighting code to the separate file.
    static bool lighting_enabled = false;
    static Framebuffer masking;
    static Framebuffer raymarching;
    static UniformBuffer lights_uniform_buffer;
    static LightSourceArray light_sources;

    bool is_lighting_enabled()
    {
        return lighting_enabled;
    }

    void enable_lighting()
    {
        lighting_enabled = true;

        masking = framebuffer_create(active_window->width, active_window->height);
        raymarching = framebuffer_create(active_window->width, active_window->height);

        if (light_sources.data == nullptr)
        {
            light_sources.data = arena_push_uninitialized<LightSource>(&memory.persistent, MAX_LIGHT_SOURCES);
            light_sources.max_count = MAX_LIGHT_SOURCES;
            light_sources.count = 0;

        }

        lights_uniform_buffer = uniformbuffer_create(MAX_LIGHT_SOURCES * sizeof(LightSource), 0, "Lights");
        uniformbuffer_bind_to_shader(&lights_uniform_buffer, raymarching_shader);
        uniformbuffer_bind_to_shader(&lights_uniform_buffer, shadowcasting_shader);
    }

    void disable_lighting()
    {
        lighting_enabled = false;

        framebuffer_destroy(&masking);
        framebuffer_destroy(&raymarching);

        light_sources.count = 0;

        uniformbuffer_destroy(&lights_uniform_buffer);
    }

    void add_light_source(LightSource source)
    {
        if (lighting_enabled)
        {
            APORIA_ASSERT(light_sources.count < light_sources.max_count);
            light_sources.data[light_sources.count] = source;
            light_sources.count += 1;
        }
    }

    void rendering_init(MemoryArena* arena)
    {
        rendering_queue = renderqueue_create(arena, MAX_RENDERING_QUEUE_SIZE);

        // Set VertexArray for Quads
        {
            VertexArray* quads = get_vao_from_buffer(BufferType::Quads);
            *quads = vertexarray_create(4, 6);
            vertexarray_bind(quads);

            VertexBuffer quads_vbo;
            quads_vbo = vertexbuffer_create(arena, MAX_OBJECTS_PER_DRAW_CALL * 4, 4);
            vertexbuffer_add_layout(&quads_vbo);
            quads->vertex_buffer = quads_vbo;

            ScratchArena temp = scratch_begin(arena);

            u32* quad_indices = arena_push_uninitialized<u32>(temp.arena, MAX_OBJECTS_PER_DRAW_CALL * 6);
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
            quads_ibo = indexbuffer_create(MAX_OBJECTS_PER_DRAW_CALL * 6, 6, quad_indices);
            quads->index_buffer = quads_ibo;

            vertexarray_unbind();

            scratch_end(temp);
        }

        // Set VertexArray for Lines
        {
            VertexArray* lines = get_vao_from_buffer(BufferType::Lines);
            *lines = vertexarray_create(2, 2);
            vertexarray_bind(lines);

            VertexBuffer lines_vbo;
            lines_vbo = vertexbuffer_create(arena, MAX_OBJECTS_PER_DRAW_CALL * 2, 2);
            vertexbuffer_add_layout(&lines_vbo);
            lines->vertex_buffer = lines_vbo;

            ScratchArena temp = scratch_begin(arena);

            u32* line_indices = arena_push_uninitialized<u32>(temp.arena, MAX_OBJECTS_PER_DRAW_CALL * 2);
            for (u32 i = 0; i < MAX_OBJECTS_PER_DRAW_CALL * 2; ++i)
            {
                line_indices[i] = i;
            }

            IndexBuffer lines_ibo;
            lines_ibo = indexbuffer_create(MAX_OBJECTS_PER_DRAW_CALL * 2, 2, line_indices);
            lines->index_buffer = lines_ibo;

            vertexarray_unbind();

            scratch_end(temp);
        }

        // Setup Framebuffers
        adjust_framebuffers_to_render_surface();

#if defined(APORIA_EMSCRIPTEN)
    #define SHADERS_DIRECTORY "content/shaders_gles/"
#else
    #define SHADERS_DIRECTORY "content/shaders/"
#endif

        // Setup default shaders
        default_shader = load_shader(SHADERS_DIRECTORY "default.glsl");
        rectangle_shader = load_shader(SHADERS_DIRECTORY "rectangle.glsl");
        line_shader = load_shader(SHADERS_DIRECTORY "line.glsl");
        circle_shader = load_shader(SHADERS_DIRECTORY "circle.glsl");
        font_shader = load_shader(SHADERS_DIRECTORY "font.glsl");

        // Setup post-processing shaders
        postprocessing_shader = load_shader(SHADERS_DIRECTORY "postprocessing.glsl");

        // Setup lighting shaders
        raymarching_shader = load_shader(SHADERS_DIRECTORY "raymarching.glsl");
        shadowcasting_shader = load_shader(SHADERS_DIRECTORY "shadowcasting.glsl");

        // Setup editor grid shaders
        editor_grid_shader = load_shader(SHADERS_DIRECTORY "editor_grid.glsl");

        unbind_shader();
    }

    void rendering_deinit()
    {
        for (u64 idx = 0; idx < ARRAY_COUNT(vertex_arrays); ++idx)
        {
            vertexarray_destroy(&vertex_arrays[idx]);
        }

        framebuffer_destroy(&main_framebuffer);
        if (lighting_enabled)
        {
            disable_lighting();
        }
        remove_all_shaders();
    }

    void rendering_frame_begin()
    {
        light_sources.count = 0;
    }

    void rendering_frame_end()
    {
        framebuffer_bind(temp_framebuffer);
        framebuffer_clear(camera_config.background_color);

        const m4& view_projection_matrix = active_camera->calculate_view_projection_matrix();
        f32 camera_zoom = 1.f / active_camera->projection.zoom;

        // Initialize texture sampler
        static i32 sampler[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

        bind_shader(default_shader);
        shader_set_int_array("u_atlas", sampler, OPENGL_MAX_TEXTURE_UNITS);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(rectangle_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(line_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(circle_shader);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);

        bind_shader(font_shader);
        shader_set_int_array("u_atlas", sampler, OPENGL_MAX_TEXTURE_UNITS);
        shader_set_mat4("u_vp_matrix", view_projection_matrix);
        shader_set_float("u_camera_zoom", camera_zoom);

        if (editor_config.display_editor_grid)
        {
            bind_shader(editor_grid_shader);
            shader_set_mat4("u_vp_matrix", view_projection_matrix);

            VertexArray* quads = get_vao_from_buffer(BufferType::Quads);

            u64 idx = quads->vertex_buffer.count;
            quads->vertex_buffer.data[idx + 0] = Vertex{ v3{ -1.f, -1.f, 0.f } };
            quads->vertex_buffer.data[idx + 1] = Vertex{ v3{  1.f, -1.f, 0.f } };
            quads->vertex_buffer.data[idx + 2] = Vertex{ v3{  1.f,  1.f, 0.f } };
            quads->vertex_buffer.data[idx + 3] = Vertex{ v3{ -1.f,  1.f, 0.f } };
            quads->vertex_buffer.count += 4;

            vertexarray_render(quads);
        }

        renderqueue_flush(&rendering_queue);
        framebuffer_unbind();

        if (lighting_enabled)
        {
            //////////////////////////////////////////////////
            // Masking Shader

            // @NOTE(dubgron): Resize the viewport to the size of the window
            // before rendering the lights to keep the highest fidelity.
            glViewport(0, 0, active_window->width, active_window->height);

            framebuffer_bind(masking);
            framebuffer_clear(Color::Transparent);

            for (u64 idx = 0; idx < world.entity_count; ++idx)
            {
                const Entity& entity = world.entity_array[idx];
                if (entity_flag_is_set(entity, EntityFlag_Visible) && entity_flag_is_set(entity, EntityFlag_BlockingLight))
                {
                    draw_entity(entity);
                }
            }

            renderqueue_flush(&rendering_queue);
            framebuffer_unbind();

            DEBUG_TEXTURE(masking.color_buffer);

            //////////////////////////////////////////////////
            // Raymarching Shader

            uniformbuffer_set_data(&lights_uniform_buffer, light_sources.data, light_sources.count * sizeof(LightSource));

            u32 masking_unit = find_or_assign_texture_unit(masking.color_buffer.id);

            bind_shader(raymarching_shader);
            shader_set_mat4("u_vp_matrix", view_projection_matrix);
            shader_set_int("u_masking", masking_unit);
            shader_set_float("u_camera_zoom", camera_zoom);
            shader_set_float2("u_window_size", active_window->width, active_window->height);
            shader_set_uint("u_num_lights", light_sources.count);

            framebuffer_bind(raymarching);
            framebuffer_clear(Color::Black);
            framebuffer_flush(masking, raymarching_shader);
            framebuffer_unbind();

            DEBUG_TEXTURE(raymarching.color_buffer);

            // @NOTE(dubgron): Resize the viewport back to the size of the buffer.
            glViewport(0, 0, temp_framebuffer.color_buffer.width, temp_framebuffer.color_buffer.height);

            //////////////////////////////////////////////////
            // Shadowcasting Shader

            u32 raymarching_unit = find_or_assign_texture_unit(raymarching.color_buffer.id);

            bind_shader(shadowcasting_shader);
            shader_set_mat4("u_vp_matrix", view_projection_matrix);
            shader_set_int("u_raymarching", raymarching_unit);
            shader_set_float("u_camera_zoom", camera_zoom);
            shader_set_float2("u_window_size", active_window->width, active_window->height);
            shader_set_uint("u_num_lights", light_sources.count);

            framebuffer_bind(temp_framebuffer);
            framebuffer_flush(raymarching, shadowcasting_shader);
            framebuffer_unbind();
        }

        DEBUG_TEXTURE(temp_framebuffer.color_buffer);

        u32 temp_framebuffer_unit = find_or_assign_texture_unit(temp_framebuffer.color_buffer.id);

        bind_shader(postprocessing_shader);
        shader_set_int("u_framebuffer", temp_framebuffer_unit);

        // @NOTE(dubgron): The temporary framebuffer is here to avoid FBO's Feedback Loop.
        // See https://www.khronos.org/opengl/wiki/Framebuffer_Object#Feedback_Loops.
        framebuffer_bind(main_framebuffer);
        framebuffer_flush(temp_framebuffer, postprocessing_shader);
        framebuffer_unbind();
    }

    void rendering_ui_begin()
    {
    }

    void rendering_ui_end()
    {
        framebuffer_bind(main_framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);

        m4 screen_to_clip = glm::ortho<f32>(0.f, active_window->width, 0.f, active_window->height);

        // Initialize texture sampler
        static i32 sampler[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

        bind_shader(default_shader);
        shader_set_int_array("u_atlas", sampler, OPENGL_MAX_TEXTURE_UNITS);
        shader_set_mat4("u_vp_matrix", screen_to_clip);

        bind_shader(rectangle_shader);
        shader_set_mat4("u_vp_matrix", screen_to_clip);

        bind_shader(line_shader);
        shader_set_mat4("u_vp_matrix", screen_to_clip);

        bind_shader(circle_shader);
        shader_set_mat4("u_vp_matrix", screen_to_clip);

        bind_shader(font_shader);
        shader_set_int_array("u_atlas", sampler, OPENGL_MAX_TEXTURE_UNITS);
        shader_set_mat4("u_vp_matrix", screen_to_clip);
        shader_set_float("u_camera_zoom", 1.f);

        renderqueue_flush(&rendering_queue);
        framebuffer_unbind();
    }

    void rendering_flush_to_screen()
    {
        f32 render_aspect_ratio = (f32)main_framebuffer.color_buffer.width / (f32)main_framebuffer.color_buffer.height;
        f32 window_aspect_ratio = (f32)active_window->width / (f32)active_window->height;

        i32 offset_x, offset_y, render_width, render_height;

        if (render_aspect_ratio > window_aspect_ratio)
        {
            render_width = active_window->width;
            render_height = active_window->width / render_aspect_ratio;

            offset_x = 0;
            offset_y = (active_window->height - render_height) / 2;
        }
        else
        {
            render_width = active_window->height * render_aspect_ratio;
            render_height = active_window->height;

            offset_x = (active_window->width - render_width) / 2;
            offset_y = 0;
        }

        framebuffer_unbind();
        framebuffer_clear(Color::Black);

#if defined(APORIA_EMSCRIPTEN)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, main_framebuffer.framebuffer_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, main_framebuffer.color_buffer.width, main_framebuffer.color_buffer.height,
            offset_x, offset_y, offset_x + render_width, offset_y + render_height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else
        glBlitNamedFramebuffer(main_framebuffer.framebuffer_id, 0,
            0, 0, main_framebuffer.color_buffer.width, main_framebuffer.color_buffer.height,
            offset_x, offset_y, offset_x + render_width, offset_y + render_height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif
    }

    void draw_entity(const Entity& entity)
    {
        f32 sin = std::sin(entity.rotation);
        f32 cos = std::cos(entity.rotation);

        v3 right_offset       = v3{ cos, sin, 0.f } * entity.width * entity.scale.x;
        v3 up_offset          = v3{ -sin, cos, 0.f } * entity.height * entity.scale.y;

        v3 offset_from_center = right_offset * entity.center_of_rotation.x + up_offset * entity.center_of_rotation.y;
        v3 base_offset        = v3{ entity.position, entity.z } - offset_from_center;

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

        if (Texture* texture = get_texture(entity.texture.texture_index))
        {
            key.texture_id              = texture->id;

            key.vertex[0].tex_coord     = v2{ entity.texture.u.x, entity.texture.v.y };
            key.vertex[1].tex_coord     = entity.texture.v;
            key.vertex[2].tex_coord     = v2{ entity.texture.v.x, entity.texture.u.y };
            key.vertex[3].tex_coord     = entity.texture.u;
        }

        renderqueue_add(&rendering_queue, key);
    }

    void draw_rectangle(v2 position, f32 width, f32 height, Color color /* = Color::White */, u32 shader_id /* = rectangle_shader */)
    {
        v3 base_offset    = v3{ position, 0.f };
        v3 right_offset   = v3{ width, 0.f, 0.f };
        v3 up_offset      = v3{ 0.f, height, 0.f };

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

        renderqueue_add(&rendering_queue, key);
    }

    void draw_line(v2 begin, v2 end, f32 thickness /* = 1.f */, Color color /* = Color::White */, u32 shader_id /* = line_shader */)
    {
        v2 direction = glm::normalize(end - begin);
        v2 normal = v2{ -direction.y, direction.x };

        RenderQueueKey key;
        key.buffer                  = BufferType::Quads;
        key.shader_id               = shader_id;

        key.vertex[0].position      = v3{ begin, 0.f };
        key.vertex[0].color         = color;
        key.vertex[0].tex_coord     = normal;
        key.vertex[0].additional    = thickness;

        key.vertex[1].position      = v3{ begin, 0.f };
        key.vertex[1].color         = color;
        key.vertex[1].tex_coord     = -normal;
        key.vertex[1].additional    = thickness;

        key.vertex[2].position      = v3{ end, 0.f };
        key.vertex[2].color         = color;
        key.vertex[2].tex_coord     = -normal;
        key.vertex[2].additional    = thickness;

        key.vertex[3].position      = v3{ end, 0.f };
        key.vertex[3].color         = color;
        key.vertex[3].tex_coord     = normal;
        key.vertex[3].additional    = thickness;

        renderqueue_add(&rendering_queue, key);
    }

    void draw_circle(v2 position, f32 radius, Color color /* = Color::White */, u32 shader_id /* = circle_shader */)
    {
        v3 base_offset          = v3{ position, 0.f };
        v3 right_half_offset    = v3{ -radius, 0.f, 0.f };
        v3 up_half_offset       = v3{ 0.f, radius, 0.f };

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

        renderqueue_add(&rendering_queue, key);
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

        Texture* texture = get_texture(font.atlas.source);
        if (!texture)
        {
            return;
        }

        // Adjust text scaling by the predefined atlas font size
        f32 effective_font_size   = text.font_size / font.atlas.font_size;
        f32 screen_px_range       = font.atlas.distance_range * effective_font_size;

        f32 sin = std::sin(text.rotation);
        f32 cos = std::cos(text.rotation);

        u64 line_count = 1;
        for (u64 idx = 0; idx < text.caption.length; ++idx)
        {
            if (text.caption.data[idx] == '\n')
            {
                line_count += 1;
            }
        }

        ScratchArena temp = scratch_begin();

        f32* line_alignments = arena_push<f32>(temp.arena, line_count);
        f32 max_line_alignment = 0.f;

        // Calculate the alignment of every text line
        {
            u64 current_line = 0;
            for (u64 idx = 0; idx < text.caption.length; ++idx)
            {
                u8 character = text.caption.data[idx];
                if (idx > 0)
                {
                    u8 prev_character = text.caption.data[idx - 1];

                    for (u64 idx = 0; idx < font.kerning_count; ++idx)
                    {
                        const Kerning& kerning = font.kerning[idx];
                        if (kerning.unicode_1 == prev_character && kerning.unicode_2 == character)
                        {
                            line_alignments[current_line] += kerning.advance;
                        }
                    }

                    for (u64 idx = 0; idx < font.glyphs_count; ++idx)
                    {
                        const Glyph& glyph = font.glyphs[idx];
                        if (glyph.unicode == prev_character)
                        {
                            line_alignments[current_line] += glyph.advance;
                        }
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

            u8 last_character = text.caption.data[text.caption.length - 1];
            for (u64 idx = 0; idx < font.glyphs_count; ++idx)
            {
                const Glyph& glyph = font.glyphs[idx];
                if (glyph.unicode == last_character)
                {
                    line_alignments[line_count - 1] += glyph.advance;
                }
            }

            max_line_alignment = max(max_line_alignment, line_alignments[line_count - 1]);

            static constexpr f32 align_blend[] = { 0.f, 0.5f, 1.f };
            u64 alignment_id = static_cast<u64>(text.alignment);

            for (u64 idx = 0; idx < line_count; ++idx)
            {
                line_alignments[idx] = (max_line_alignment - line_alignments[idx]) * align_blend[alignment_id];
            }
        }

        // @TODO(dubgron): Fix this. Right now we don't load x-height from font, so we have to approximate it.
        f32 x_height = font.metrics.line_height * 0.65f;
        f32 total_text_height = (line_count - 1) * font.metrics.line_height + x_height;
        v2 center_offset = v2{ max_line_alignment, total_text_height } * text.center_of_rotation;

        u64 current_line = 0;
        v2 advance{ 0.f, total_text_height - x_height };
        for (u64 idx = 0; idx < text.caption.length; ++idx)
        {
            u8 character = text.caption.data[idx];

            if (idx > 0)
            {
                u8 prev_character = text.caption.data[idx - 1];

                for (u64 idx = 0; idx < font.kerning_count; ++idx)
                {
                    const Kerning& kerning = font.kerning[idx];
                    if (kerning.unicode_1 == prev_character && kerning.unicode_2 == character)
                    {
                        advance.x += kerning.advance;
                    }
                }

                for (u64 idx = 0; idx < font.glyphs_count; ++idx)
                {
                    const Glyph& glyph = font.glyphs[idx];
                    if (glyph.unicode == prev_character)
                    {
                        advance.x += glyph.advance;
                    }
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
            else
            {
                Glyph* glyph = nullptr;
                for (u64 idx = 0; idx < font.glyphs_count; ++idx)
                {
                    if (font.glyphs[idx].unicode == character)
                    {
                        glyph = &font.glyphs[idx];
                        break;
                    }
                }

                if (!glyph)
                {
                    break;
                }

                const GlyphBounds& atlas_bounds = glyph->atlas_bounds;
                const GlyphBounds& plane_bounds = glyph->plane_bounds;

                v2 texture_size   = v2{ texture->width, texture->height };

                v2 tex_coord_u    = v2{ atlas_bounds.left, atlas_bounds.top } / texture_size;
                v2 tex_coord_v    = v2{ atlas_bounds.right, atlas_bounds.bottom } / texture_size;

                f32 width         = (atlas_bounds.right - atlas_bounds.left) * effective_font_size;
                f32 height        = (atlas_bounds.bottom - atlas_bounds.top) * effective_font_size;

                // @NOTE(dubgron): We flip the sign of plane_bounds.bottom because
                // the plane_bounds lives in a space where the y-axis goes downwards.
                v2 plane_offset   = v2{ plane_bounds.left, -plane_bounds.bottom };
                v2 align_offset   = v2{ line_alignments[current_line], 0.f };
                v2 line_offset    = advance + plane_offset + align_offset - center_offset;

                f32 rotated_x     = cos * line_offset.x - sin * line_offset.y;
                f32 rotated_y     = sin * line_offset.x + cos * line_offset.y;

                v2 base_offset    = text.position + v2{ rotated_x, rotated_y } * text.font_size;
                v2 right_offset   = v2{ cos , sin } * width;
                v2 up_offset      = v2{ -sin, cos } * height;

                RenderQueueKey key;
                key.buffer                  = BufferType::Quads;
                key.shader_id               = text.shader_id;
                key.texture_id              = texture->id;

                key.vertex[0].position      = v3{ base_offset, 0.f };
                key.vertex[0].tex_coord     = v2{ tex_coord_u.x, tex_coord_v.y };
                key.vertex[0].color         = text.color;
                key.vertex[0].additional    = screen_px_range;

                key.vertex[1].position      = v3{ base_offset + right_offset, 0.f };
                key.vertex[1].tex_coord     = tex_coord_v;
                key.vertex[1].color         = text.color;
                key.vertex[1].additional    = screen_px_range;

                key.vertex[2].position      = v3{ base_offset + right_offset + up_offset, 0.f };
                key.vertex[2].tex_coord     = v2{ tex_coord_v.x, tex_coord_u.y };
                key.vertex[2].color         = text.color;
                key.vertex[2].additional    = screen_px_range;

                key.vertex[3].position      = v3{ base_offset + up_offset, 0.f };
                key.vertex[3].tex_coord     = tex_coord_u;
                key.vertex[3].color         = text.color;
                key.vertex[3].additional    = screen_px_range;

                renderqueue_add(&rendering_queue, key);
            }
        }

        scratch_end(temp);
    }

    void get_size_of_render_surface(i32* width, i32* height)
    {
        if (rendering_config.is_using_custom_resolution())
        {
            *width = rendering_config.custom_resolution_width;
            *height = rendering_config.custom_resolution_height;
        }
        else
        {
            APORIA_ASSERT(active_window);
            *width = active_window->width;
            *height = active_window->height;
        }

        APORIA_ASSERT(*width > 0 && *height > 0);
    }

    void adjust_framebuffers_to_render_surface()
    {
        i32 render_width, render_height;
        get_size_of_render_surface(&render_width, &render_height);

        framebuffer_resize(&main_framebuffer, render_width, render_height);
        framebuffer_resize(&temp_framebuffer, render_width, render_height);

        glViewport(0, 0, render_width, render_height);

        if (lighting_enabled)
        {
            framebuffer_resize(&masking, active_window->width, active_window->height);
            framebuffer_resize(&raymarching, active_window->width, active_window->height);
        }
    }
}
