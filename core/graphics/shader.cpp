#include "shader.hpp"

#include "graphics/opengl.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    Shader::Shader(Logger& logger)
        : _logger(logger)
    {
        _program = glCreateProgram();
    }

    void Shader::bind() const
    {
        glUseProgram(_program);
    }

    void Shader::unbind() const
    {
        glUseProgram(0);
    }

    bool Shader::load_shader(const std::string& path, Type type)
    {
        int32_t shader_type;
        switch (type)
        {
            case Type::Fragment:
                shader_type = GL_FRAGMENT_SHADER; break;
            case Type::Vertex:
                shader_type = GL_VERTEX_SHADER; break;
            default:
                _logger.log(LOG_ERROR) << "Unsupported type of shaders!";
                return false;
        }

        unsigned int shader = glCreateShader(shader_type);

        std::string contents_string = read_file(path);
        auto contents = contents_string.c_str();

        glShaderSource(shader, 1, &contents, nullptr);
        glCompileShader(shader);

        int32_t results;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &results);
        if (results == GL_FALSE)
        {
            int32_t length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

            std::vector<GLchar> msg(length);
            glGetShaderInfoLog(shader, length, &length, &msg[0]);

            _logger.log(LOG_ERROR) << std::string{ &msg[0] };

            glDeleteProgram(shader);
        }

        _shaders.push_back(shader);

        return true;
    }

    void Shader::compile()
    {
        for (uint32_t shader : _shaders)
            glAttachShader(_program, shader);

        glLinkProgram(_program);
        glValidateProgram(_program);

        for (uint32_t shader : _shaders)
        {
            glDetachShader(_program, shader);
            glDeleteShader(shader);
        }
    }

    void Shader::set_float(const std::string& name, float value)
    {
        glUniform1f(_location(name), value);
    }

    void Shader::set_float2(const std::string& name, glm::vec2 value)
    {
        glUniform2f(_location(name), value.x, value.y);
    }

    void Shader::set_float3(const std::string& name, glm::vec3 value)
    {
        glUniform3f(_location(name), value.x, value.y, value.z);
    }

    void Shader::set_float4(const std::string& name, glm::vec4 value)
    {
        glUniform4f(_location(name), value.x, value.y, value.z, value.w);
    }

    void Shader::set_float_array(const std::string& name, float* value, int32_t count)
    {
        glUniform1fv(_location(name), count, value);
    }

#   if !defined(APORIA_EMSCRIPTEN)
        void Shader::set_double(const std::string& name, double value)
        {
            glUniform1d(_location(name), value);
        }

        void Shader::set_double2(const std::string& name, glm::dvec2 value)
        {
            glUniform2d(_location(name), value.x, value.y);
        }

        void Shader::set_double3(const std::string & name, glm::dvec3 value)
        {
            glUniform3d(_location(name), value.x, value.y, value.z);
        }

        void Shader::set_double4(const std::string & name, glm::dvec4 value)
        {
            glUniform4d(_location(name), value.x, value.y, value.z, value.w);
        }

        void Shader::set_double_array(const std::string& name, double* value, int32_t count)
        {
            glUniform1dv(_location(name), count, value);
        }
#   endif

    void Shader::set_int(const std::string& name, int32_t value)
    {
        glUniform1i(_location(name), value);
    }

    void Shader::set_int2(const std::string& name, glm::ivec2 value)
    {
        glUniform2i(_location(name), value.x, value.y);
    }

    void Shader::set_int3(const std::string & name, glm::ivec3 value)
    {
        glUniform3i(_location(name), value.x, value.y, value.z);
    }

    void Shader::set_int4(const std::string & name, glm::ivec4 value)
    {
        glUniform4i(_location(name), value.x, value.y, value.z, value.w);
    }

    void Shader::set_int_array(const std::string& name, int* value, int32_t count)
    {
        glUniform1iv(_location(name), count, value);
    }

    void Shader::set_uint(const std::string & name, uint32_t value)
    {
        glUniform1ui(_location(name), value);
    }

    void Shader::set_uint2(const std::string & name, glm::uvec2 value)
    {
        glUniform2ui(_location(name), value.x, value.y);
    }

    void Shader::set_uint3(const std::string & name, glm::uvec3 value)
    {
        glUniform3ui(_location(name), value.x, value.y, value.z);
    }

    void Shader::set_uint4(const std::string & name, glm::uvec4 value)
    {
        glUniform4ui(_location(name), value.x, value.y, value.z, value.w);
    }

    void Shader::set_uint_array(const std::string& name, uint32_t* value, int32_t count)
    {
        glUniform1uiv(_location(name), count, value);
    }

    void Shader::set_mat2(const std::string & name, glm::mat2 value, bool transpose, int32_t count)
    {
        glUniformMatrix2fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void Shader::set_mat3(const std::string & name, glm::mat3 value, bool transpose, int32_t count)
    {
        glUniformMatrix3fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void Shader::set_mat4(const std::string& name, glm::mat4 value, bool transpose, int32_t count)
    {
        glUniformMatrix4fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    int32_t Shader::_location(const std::string& name)
    {
        int32_t location;

        if (!_locations.contains(name))
        {
            location = glGetUniformLocation(_program, name.c_str());
            if (location == -1)
                _logger.log(LOG_ERROR) << "'" << name << "' does not correspond to an active uniform variable in program " << _program << "!";
            else
                _locations.emplace(name, location);
        }
        else
            location = _locations.at(name);

        return location;
    }
}
