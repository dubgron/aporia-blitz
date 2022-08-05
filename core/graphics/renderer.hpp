#pragma once

#include <functional>
#include <utility>
#include <vector>

#include <glm/matrix.hpp>

#include "event_manager.hpp"
#include "logger.hpp"
#include "shader_manager.hpp"
#include "window.hpp"
#include "components/transform2d.hpp"
#include "configs/window_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/light.hpp"
#include "graphics/render_queue.hpp"
#include "graphics/vertex.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/drawables/group.hpp"

namespace Aporia
{
    class Renderer final
    {
        static constexpr size_t MAX_QUEUE = 100000u;

    public:
        Renderer(Logger& logger, ShaderManager& shaders, EventManager& events, WindowConfig& config);

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

        /* Predefined shaders */
        static ShaderRef default_shader;
        static ShaderRef font_shader;
        static ShaderRef postprocessing_shader;

        LightRenderer lights;

    private:
        void _flush_queue();
        void _flush_framebuffer(const Framebuffer& framebuffer, Shader program_id);
        void _flush_buffer(BufferType buffer, Shader program_id);

        void _on_resize(Window& window, uint32_t width, uint32_t height);

    private:
        Logger& _logger;
        ShaderManager& _shaders;

        RenderQueue _render_queue;

        std::vector<VertexArray> _vertex_arrays;
        Framebuffer _framebuffer;

        std::vector<Transform2D> _transformation_stack;
    };
}
