#pragma once

#include <array>

#include "aporia_textures.hpp"
#include "aporia_types.hpp"
#include "components/color.hpp"
#include "graphics/vertex.hpp"

namespace Aporia
{
    class Framebuffer final
    {
    public:
        Framebuffer();
        ~Framebuffer();

        void bind() const;
        void unbind() const;

        /* TODO: Make clear more customizable */
        void clear(Color color = Colors::Black);

        /* TODO: Make crate_framebuffer more customizable */
        void create_framebuffer(i32 width, i32 height);

        const Texture& get_color_buffer() const;
        const std::array<Vertex, 4>& get_vertices() const;

    private:
        u32 _framebuffer_id = 0;
        u32 _renderbuffer_id = 0;

        Texture _color_buffer;
        std::array<Vertex, 4> _vertex;
    };
}
