#pragma once

#include <string>
#include <type_traits>

#include "aporia_shaders.hpp"
#include "aporia_types.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    template<typename T>
    concept BufferData = requires (T data)
    {
        reinterpret_cast<const void*>(data);
    };

    template<BufferData T, u64 Binding>
    class UniformBuffer final
    {
    public:
        void init(std::string name)
        {
            _name = std::move(name);

            glGenBuffers(1, &_id);

            glBindBuffer(GL_UNIFORM_BUFFER, _id);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBufferBase(GL_UNIFORM_BUFFER, Binding, _id);
        }

        void deinit()
        {
            glDeleteBuffers(1, &_id);
        }

        void bind_to_shader(ShaderID program_id)
        {
            const u32 buffer_index = glGetUniformBlockIndex(program_id, _name.c_str());
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
        u32 _id = 0;
        std::string _name;
    };
}
