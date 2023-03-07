#pragma once

#include <vector>

#include "aporia_types.hpp"
#include "components/transform2d.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/light.hpp"
#include "graphics/render_queue.hpp"
#include "graphics/vertex_array.hpp"

namespace Aporia
{
    class Camera;
    class Window;
    struct Circle2D;
    struct Line2D;
    struct Rectangle2D;
    struct Sprite;
    struct Text;
    struct WindowConfig;

    class Renderer final
    {
        static constexpr u64 MAX_QUEUE = 100000;

    public:
        void init(u32 width, u32 height);
        void deinit();

        void begin(const Window& window, const Camera& camera);
        void end(Color color = Colors::Black);

        void draw(const Group& group);
        void draw(const Sprite& sprite);
        void draw(const Rectangle2D& rect);
        void draw(const Line2D& line);
        void draw(const Circle2D& circle);
        void draw(const Text& text);

        void push_transform(const Transform2D& transform);
        void pop_transform();

        void on_window_resize(u32 width, u32 height);

    public:
        /* Predefined shaders */
        static ShaderID default_shader;
        static ShaderID font_shader;
        static ShaderID postprocessing_shader;

        LightRenderer lights;

    private:
        void _flush_queue();
        void _flush_framebuffer(const Framebuffer& framebuffer, ShaderID program_id);
        void _flush_buffer(BufferType buffer, ShaderID program_id);

    private:
        RenderQueue _render_queue;

        std::vector<VertexArray> _vertex_arrays;
        Framebuffer _framebuffer;

        std::vector<Transform2D> _transformation_stack;
    };
}
