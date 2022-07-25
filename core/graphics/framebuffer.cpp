#include "framebuffer.hpp"

#include <cstddef>

#include "graphics/opengl.hpp"

namespace Aporia
{
    Framebuffer::Framebuffer()
    {
        /* Setup vertices */
        _vertex[0].position = glm::vec3{ -1.0f, -1.0f, 0.0f };
        _vertex[0].tex_coord = glm::vec2{ 0.0f, 0.0f };

        _vertex[1].position = glm::vec3{ 1.0f, -1.0f, 0.0f };
        _vertex[1].tex_coord = glm::vec2{ 1.0f, 0.0f };

        _vertex[2].position = glm::vec3{ 1.0f, 1.0f, 0.0f };
        _vertex[2].tex_coord = glm::vec2{ 1.0f, 1.0f };

        _vertex[3].position = glm::vec3{ -1.0f, 1.0f, 0.0f };
        _vertex[3].tex_coord = glm::vec2{ 0.0f, 1.0f };
    }

    Framebuffer::~Framebuffer()
    {
        uint32_t color_buffer_id = static_cast<uint32_t>(_color_buffer.id);
        glDeleteTextures(1, &color_buffer_id);
        glDeleteRenderbuffers(1, &_rbo);
        glDeleteFramebuffers(1, &_fbo);
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glEnable(GL_DEPTH_TEST);
    }

    void Framebuffer::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
    }

    void Framebuffer::create_framebuffer(int32_t width, int32_t height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        /* Delete potential previous buffers and textures */
        uint32_t color_buffer_id = static_cast<uint32_t>(_color_buffer.id);
        glDeleteTextures(1, &color_buffer_id);
        glDeleteRenderbuffers(1, &_rbo);
        glDeleteFramebuffers(1, &_fbo);

        /* Create new buffers and textures */
        glGenFramebuffers(1, &_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

        glGenTextures(1, &color_buffer_id);
        glActiveTexture(GL_TEXTURE0 + color_buffer_id);
        glBindTexture(GL_TEXTURE_2D, color_buffer_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);

        glGenRenderbuffers(1, &_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        _color_buffer = Texture::Origin{
            .id = static_cast<texture_id>(color_buffer_id),
            .width = width,
            .height = height,
            .channels = 3 };

        for (size_t i = 0; i < _vertex.size(); ++i)
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
