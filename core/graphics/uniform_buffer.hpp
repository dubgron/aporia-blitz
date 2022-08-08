#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <type_traits>

#include "graphics/opengl.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    template<typename T>
    concept BufferData = requires (T data)
    {
        reinterpret_cast<const void*>(data);
    };

    template<BufferData T, size_t Binding>
    class UniformBuffer final
    {
    public:
        UniformBuffer(std::string name)
            : _name( std::move(name) )
        {
            glGenBuffers(1, &_id);

            glBindBuffer(GL_UNIFORM_BUFFER, _id);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), NULL, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBufferBase(GL_UNIFORM_BUFFER, Binding, _id);
        }

        ~UniformBuffer()
        {
            glDeleteBuffers(1, &_id);
        }

        void bind_to_shader(Shader program_id)
        {
            const uint32_t buffer_index = glGetUniformBlockIndex(program_id, _name.c_str());
            glUniformBlockBinding(program_id, buffer_index, Binding);
        }

        template<BufferData U>
        void set_data(U data)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, _id);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), reinterpret_cast<const void*>(data));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    private:
        uint32_t _id = 0;
        std::string _name;
    };
}
