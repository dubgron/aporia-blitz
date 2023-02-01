#include "framebuffer.hpp"

#include <cstddef>

#include "platform/opengl.hpp"

namespace Aporia
{
    Framebuffer::Framebuffer()
    {
        /* Setup vertices */
        _vertex[0].position = v3{ -1.f, -1.f, 0.f };
        _vertex[0].tex_coord = v2{ 0.f, 0.f };

        _vertex[1].position = v3{ 1.f, -1.f, 0.f };
        _vertex[1].tex_coord = v2{ 1.f, 0.f };

        _vertex[2].position = v3{ 1.f, 1.f, 0.f };
        _vertex[2].tex_coord = v2{ 1.f, 1.f };

        _vertex[3].position = v3{ -1.f, 1.f, 0.f };
        _vertex[3].tex_coord = v2{ 0.f, 1.f };
    }

    Framebuffer::~Framebuffer()
    {
        u32 color_buffer_id = static_cast<u32>(_color_buffer.id);
        glDeleteTextures(1, &color_buffer_id);
        glDeleteRenderbuffers(1, &_renderbuffer_id);
        glDeleteFramebuffers(1, &_framebuffer_id);
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);
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

    void Framebuffer::create_framebuffer(i32 width, i32 height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        /* Delete potential previous buffers and textures */
        u32 color_buffer_id = static_cast<u32>(_color_buffer.id);
        glDeleteTextures(1, &color_buffer_id);
        glDeleteRenderbuffers(1, &_renderbuffer_id);
        glDeleteFramebuffers(1, &_framebuffer_id);

        /* Create new buffers and textures */
        glGenFramebuffers(1, &_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);

        glGenTextures(1, &color_buffer_id);
        glActiveTexture(GL_TEXTURE0 + color_buffer_id);
        glBindTexture(GL_TEXTURE_2D, color_buffer_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);

        glGenRenderbuffers(1, &_renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderbuffer_id);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        _color_buffer = Texture::Origin{
            .id = static_cast<texture_id>(color_buffer_id),
            .width = width,
            .height = height,
            .channels = 3 };

        for (u64 i = 0; i < _vertex.size(); ++i)
        {
            _vertex[i].tex_id = _color_buffer.id;
        }
    }

    const Texture::Origin& Framebuffer::get_color_buffer() const
    {
        return _color_buffer;
    }

    const std::array<Vertex, 4>& Framebuffer::get_vertices() const
    {
        return _vertex;
    }
}
