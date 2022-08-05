#pragma once

#include <array>
#include <cstdint>

#include "components/color.hpp"
#include "components/texture.hpp"
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
        void create_framebuffer(int32_t width, int32_t height);

        const Texture::Origin& get_color_buffer() const;
        const std::array<Vertex, 4>& get_vertices() const;

    private:
        uint32_t _fbo = 0u;
        uint32_t _rbo = 0u;

        Texture::Origin _color_buffer;
        std::array<Vertex, 4> _vertex;
    };
}
