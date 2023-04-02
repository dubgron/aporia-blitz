#include "aporia_rendering.hpp"

#include "window.hpp"
#include "graphics/camera.hpp"
#include "graphics/drawables/group.hpp"
#include "utils/math.hpp"

#if defined(APORIA_DEBUG)
    #define DEBUG_TEXTURE(texture) \
        ImGui::Begin("DEBUG | Textures"); \
        ImGui::Text("ID: %d, Size: %d x %d", texture.id, texture.width, texture.height); \
        ImGui::Image((void*)(u64)texture.id, ImVec2{ (f32)texture.width, (f32)texture.height }, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f }); \
        ImGui::End();
#else
    #define DEBUG_TEXTURE
#endif

namespace Aporia
{
    static constexpr u64 MAX_RENDERING_QUEUE_SIZE = 100000;
    static constexpr u64 MAX_LIGHT_SOURCES = 1000;

    ShaderID default_shader = 0;
    ShaderID font_shader = 0;
    ShaderID postprocessing_shader = 0;
    ShaderID raymarching_shader = 0;
    ShaderID shadowcasting_shader = 0;

    static Framebuffer main_framebuffer;

    static std::vector<RenderQueueKey> rendering_queue;
    static std::vector<VertexArray> vertex_arrays;
    static std::vector<Transform2D> transformation_stack;

    // @TODO(dubgron): Move the lighting code to the separate file.
    static bool lighting_enabled = false;
    static Framebuffer masking;
    static Framebuffer raymarching;
    static std::vector<LightSource> light_sources;
    static Group light_blockers;
    static UniformBuffer lights_uniform_buffer;

    bool operator<(const RenderQueueKey& key1, const RenderQueueKey& key2) noexcept
    {
        return key1.vertex[0].position.z < key2.vertex[0].position.z ||
            (key1.vertex[0].position.z == key2.vertex[0].position.z && key1.buffer < key2.buffer) ||
            (key1.vertex[0].position.z == key2.vertex[0].position.z && key1.buffer == key2.buffer && key1.program_id < key2.program_id);
    }

    void IndexBuffer::init(u32 in_max_objects, u32 in_count, const std::vector<u32>& indices)
    {
        max_objects = in_max_objects;
        count = in_count;

        glGenBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);

        const u64 buffer_size = in_max_objects * in_count * sizeof(u32);

#if defined(APORIA_EMSCRIPTEN)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices.data(), GL_STATIC_DRAW);
#else
        glNamedBufferData(id, buffer_size, indices.data(), GL_STATIC_DRAW);
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

    void VertexBuffer::init(u32 in_max_objects, u32 in_count)
    {
        max_objects = in_max_objects;
        count = in_count;

        glGenBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);

        const u64 buffer_size = in_max_objects * in_count * sizeof(Vertex);

#if defined(APORIA_EMSCRIPTEN)
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
#else
        glNamedBufferData(id, buffer_size, nullptr, GL_DYNAMIC_DRAW);
#endif

        data.reserve(in_max_objects * in_count);
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
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(Vertex), data.data());
        unbind();
#else
        glNamedBufferSubData(id, 0, data.size() * sizeof(Vertex), data.data());
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

        const u32 count = index_buffer.count * vertex_buffer.data.size() / vertex_buffer.count;
        glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);

        index_buffer.unbind();
        this->unbind();

        vertex_buffer.data.clear();
    }

    void UniformBuffer::init(u32 in_max_size, u32 in_binding_index, std::string_view in_block_name)
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

    void UniformBuffer::bind_to_shader(ShaderID program_id)
    {
        const u32 buffer_index = glGetUniformBlockIndex(program_id, block_name.data());
        glUniformBlockBinding(program_id, buffer_index, binding_index);
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

        /* Delete potential previous buffers and textures */
        destroy();

        /* Create new buffers and textures */
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

        /* Setup vertices */
        vertices[0].position = v3{ -1.f, -1.f, 0.f };
        vertices[0].tex_coord = v2{ 0.f, 0.f };
        vertices[0].tex_id = color_buffer.id;

        vertices[1].position = v3{ 1.f, -1.f, 0.f };
        vertices[1].tex_coord = v2{ 1.f, 0.f };
        vertices[1].tex_id = color_buffer.id;

        vertices[2].position = v3{ 1.f, 1.f, 0.f };
        vertices[2].tex_coord = v2{ 1.f, 1.f };
        vertices[2].tex_id = color_buffer.id;

        vertices[3].position = v3{ -1.f, 1.f, 0.f };
        vertices[3].tex_coord = v2{ 0.f, 1.f };
        vertices[3].tex_id = color_buffer.id;
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
        glEnable(GL_DEPTH_TEST);
    }

    void Framebuffer::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
    }

    void Framebuffer::clear(Color color /* = Colors::Black */)
    {
        glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static void flush_buffer(BufferType buffer, ShaderID program_id)
    {
        bind_shader(program_id);

        const u8 buffer_index = std::to_underlying(buffer);
        vertex_arrays[buffer_index].render();
    }

    static void flush_rendering_queue()
    {
        if (!rendering_queue.empty())
        {
            std::sort(rendering_queue.begin(), rendering_queue.end());

            RenderQueueKey prev_key = rendering_queue[0];
            for (RenderQueueKey& key : rendering_queue)
            {
                if (key.program_id != prev_key.program_id || key.buffer != prev_key.buffer)
                {
                    flush_buffer(prev_key.buffer, prev_key.program_id);
                }

                const u8 buffer_index = std::to_underlying(key.buffer);
                VertexBuffer& vertex_buffer = vertex_arrays[buffer_index].vertex_buffer;
                for (u64 i = 0; i < vertex_buffer.count; ++i)
                {
                    vertex_buffer.data.push_back( key.vertex[i] );
                }

                prev_key = key;
            }

            flush_buffer(prev_key.buffer, prev_key.program_id);

            rendering_queue.clear();
        }
    }

    static void flush_framebuffer(const Framebuffer& framebuffer, ShaderID program_id)
    {
        constexpr u8 buffer_index = std::to_underlying(BufferType::Quads);
        VertexBuffer& vertex_buffer = vertex_arrays[buffer_index].vertex_buffer;
        for (Vertex vertex : framebuffer.vertices)
        {
            vertex_buffer.data.push_back( vertex );
        }

        flush_buffer(BufferType::Quads, program_id);
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

        light_sources.reserve(MAX_LIGHT_SOURCES);

        lights_uniform_buffer.init(MAX_LIGHT_SOURCES * sizeof(LightSource), 0, "Lights");
        lights_uniform_buffer.bind_to_shader(raymarching_shader);
        lights_uniform_buffer.bind_to_shader(shadowcasting_shader);
    }

    void disable_lighting()
    {
        lighting_enabled = false;

        masking.destroy();
        raymarching.destroy();

        lights_uniform_buffer.deinit();
    }

    void add_light_source(LightSource light_source)
    {
        light_sources.push_back(light_source);
    }

    void add_light_blocker(Group& light_blocker)
    {
        light_blockers.add(light_blocker);
    }

    void rendering_init(u32 width, u32 height)
    {
        transformation_stack.reserve(10);
        transformation_stack.emplace_back();

        rendering_queue.reserve(MAX_RENDERING_QUEUE_SIZE);

        vertex_arrays.reserve(2);

        /* Set VertexArray for opaque Quads */
        VertexArray& quads = vertex_arrays.emplace_back();
        quads.init(4, 6);
        quads.bind();

        VertexBuffer quads_vbo;
        quads_vbo.init(MAX_RENDERING_QUEUE_SIZE, 4);
        quads_vbo.add_layout();
        quads.vertex_buffer = std::move(quads_vbo);

        std::vector<u32> quad_indices(MAX_RENDERING_QUEUE_SIZE * 6);
        for (u32 i = 0, offset = 0; i < MAX_RENDERING_QUEUE_SIZE; i += 6, offset += 4)
        {
            quad_indices[  i  ] = offset + 0;
            quad_indices[i + 1] = offset + 1;
            quad_indices[i + 2] = offset + 2;

            quad_indices[i + 3] = offset + 2;
            quad_indices[i + 4] = offset + 3;
            quad_indices[i + 5] = offset + 0;
        }

        IndexBuffer quads_ibo;
        quads_ibo.init(MAX_RENDERING_QUEUE_SIZE, 6, quad_indices);
        quads.index_buffer = std::move(quads_ibo);

        quads.unbind();

        /* Set VertexArray for Lines */
        VertexArray& lines = vertex_arrays.emplace_back();
        lines.init(2, 2);
        lines.bind();

        VertexBuffer lines_vbo;
        lines_vbo.init(MAX_RENDERING_QUEUE_SIZE, 2);
        lines_vbo.add_layout();
        lines.vertex_buffer = std::move(lines_vbo);

        std::vector<u32> line_indices(MAX_RENDERING_QUEUE_SIZE * 2);
        for (u32 i = 0; i < MAX_RENDERING_QUEUE_SIZE * 2; ++i)
        {
            line_indices[i] = i;
        }

        IndexBuffer lines_ibo;
        lines_ibo.init(MAX_RENDERING_QUEUE_SIZE, 2, line_indices);
        lines.index_buffer = std::move(lines_ibo);

        lines.unbind();

        /* Setup Framebuffer */
        main_framebuffer.create(width, height);

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
        raymarching_shader = create_shader("assets/shaders/raymarching.shader");
        shadowcasting_shader = create_shader("assets/shaders/shadowcasting.shader");

        unbind_shader();
    }

    void rendering_deinit()
    {
        for (VertexArray& vao : vertex_arrays)
        {
            vao.deinit();
        }

        main_framebuffer.destroy();
        if (lighting_enabled)
        {
            disable_lighting();
        }
        remove_all_shaders();
    }

    void rendering_begin(const Window& window, const Camera& camera)
    {
        light_sources.clear();

        bind_shader(default_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());

        bind_shader(font_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());

        bind_shader(raymarching_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_int("u_masking", masking.color_buffer.id);
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());
        shader_set_float2("u_window_size", v2{ window.get_size() });

        bind_shader(shadowcasting_shader);
        shader_set_mat4("u_vp_matrix", camera.get_view_projection_matrix());
        shader_set_int("u_raymarching", raymarching.color_buffer.id);
        shader_set_float("u_camera_zoom", 1.f / camera.get_zoom());
        shader_set_float2("u_window_size", v2{ window.get_size() });

        unbind_shader();
    }

    void rendering_end(Color color /* = Colors::Black */)
    {
        main_framebuffer.bind();
        main_framebuffer.clear(color);

        flush_rendering_queue();

        main_framebuffer.unbind();

        DEBUG_TEXTURE(main_framebuffer.color_buffer);

        flush_framebuffer(main_framebuffer, postprocessing_shader);

        if (lighting_enabled)
        {
            const u32 num_lights = static_cast<u32>(light_sources.size());

            lights_uniform_buffer.set_data(light_sources.data(), num_lights * sizeof(LightSource));

            bind_shader(raymarching_shader);
            shader_set_uint("u_num_lights", num_lights);

            bind_shader(shadowcasting_shader);
            shader_set_uint("u_num_lights", num_lights);

            masking.bind();
            masking.clear(Colors::Transparent);

            draw(light_blockers);
            flush_rendering_queue();

            masking.unbind();

            DEBUG_TEXTURE(masking.color_buffer);

            raymarching.bind();
            raymarching.clear(Colors::Black);

            flush_framebuffer(masking, raymarching_shader);

            raymarching.unbind();

            DEBUG_TEXTURE(raymarching.color_buffer);

            flush_framebuffer(raymarching, shadowcasting_shader);
        }

        unbind_shader();
    }

    void draw(const Group& group)
    {
        rendering_push_transform(group.transform);

        for (const Line2D& line : group.get_container<Line2D>())
        {
            draw(line);
        }

        for (const Rectangle2D& rectangle : group.get_container<Rectangle2D>())
        {
            draw(rectangle);
        }

        for (const Circle2D& circle : group.get_container<Circle2D>())
        {
            draw(circle);
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

        rendering_pop_transform();
    }

    void draw(const Sprite& sprite)
    {
        const m4 transformation = to_mat4(transformation_stack.back() * sprite.transform);

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

        rendering_queue.push_back(key);
    }

    void draw(const Rectangle2D& rect)
    {
        const m4 transformation = to_mat4(transformation_stack.back() * rect.transform);

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

        rendering_queue.push_back(key);
    }

    void draw(const Line2D& line2d)
    {
        const m4 transformation = to_mat4(transformation_stack.back() * line2d.transform);

        const v3 base_offset = transformation[3];
        const v3 offset      = glm::mat2x4{ transformation[0], transformation[1] } * line2d.offset;

        RenderQueueKey key;
        key.buffer              = BufferType::Lines;
        key.program_id          = line2d.shader;

        key.vertex[0].position  = base_offset;
        key.vertex[0].color     = line2d.color;

        key.vertex[1].position  = base_offset + offset;
        key.vertex[1].color     = line2d.color;

        rendering_queue.push_back(key);
    }

    void draw(const Circle2D& circle)
    {
        const m4 transformation = to_mat4(transformation_stack.back() * circle.transform);

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

        rendering_queue.push_back(key);
    }

    void draw(const Text& text)
    {
        APORIA_ASSERT(text.font);

        const Font& font = *text.font;

        const v2 texture_size = v2{ font.atlas.source.width, font.atlas.source.height };
        const v2 half_pixel_offset = 0.5f / texture_size;

        /* Adjust text scaling by the predefined atlas font size */
        const Transform2D font_scale{ .scale = v2{ 1.f / font.atlas.font_size } };

        const m4 transformation = to_mat4(transformation_stack.back() * text.transform * font_scale);

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

                rendering_queue.push_back(key);
            }
        }
    }

    void rendering_push_transform(const Transform2D& transform)
    {
        transformation_stack.push_back( transformation_stack.back() * transform );
    }

    void rendering_pop_transform()
    {
        if (transformation_stack.size() > 1)
        {
            transformation_stack.pop_back();
        }
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
