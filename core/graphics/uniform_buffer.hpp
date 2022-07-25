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
    concept PODType = std::is_standard_layout_v<T>;

    template<PODType T, size_t Binding>
    class UniformBuffer final
    {
    public:
        UniformBuffer(std::string name)
            : _name( std::move(name) )
        {
            glGenBuffers(1, &_ubo);

            glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), NULL, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBufferBase(GL_UNIFORM_BUFFER, Binding, _ubo);
        }

        ~UniformBuffer()
        {
            glDeleteBuffers(1, &_ubo);
        }

        void bind_to_shader(Shader program_id)
        {
            const uint32_t buffer_index = glGetUniformBlockIndex(program_id, _name.c_str());
            glUniformBlockBinding(program_id, buffer_index, Binding);
        }

        void set_data(const T& data)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), reinterpret_cast<const void*>(data));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    private:
        uint32_t _ubo = 0u;
        std::string _name;
    };
}
