#include "shader_manager.hpp"

#include <functional>

#include "logger.hpp"
#include "configs/shader_config.hpp"
#include "utils/shader_parser.hpp"

namespace Aporia
{
    ShaderManager::ShaderManager(Logger& logger, const ShaderConfig& config)
        : _logger(logger), _config(config) {}

    ShaderManager::~ShaderManager()
    {
        for (auto& [name, program_id] : _shaders)
        {
            glDeleteProgram(program_id);
        }

        _shaders.clear();
    }

    Shader ShaderManager::create_program(const std::string& name, const std::string& filepath)
    {
        if (_shaders.contains(name))
        {
            _logger.log(LOG_ERROR) << "Shader named '" << name << "' already exits! Aborting creating a new shader program!";
            return 0;
        }

        Shader program_id = glCreateProgram();

        std::vector<ShaderRef> loaded_shaders;
        loaded_shaders.reserve(2);

        ShaderData shader_data = parse_shader(filepath);
        for (const SubShaderData& data : shader_data.subshaders)
        {
            if (Shader shader_id = _load(data.contents, data.type))
            {
                loaded_shaders.push_back(shader_id);
            }
        }

        if (loaded_shaders.size() == 0)
        {
            glDeleteProgram(program_id);
            return 0;
        }

        _link(program_id, loaded_shaders);

        _shaders[name] = program_id;
        _sources[program_id] = filepath;

        _default_invalids(shader_data.properties);
        _properties[program_id] = std::move(shader_data.properties);

        return program_id;
    }

    Shader ShaderManager::get(const std::string& name) const
    {
        auto shader = _shaders.find(name);
        if (shader == _shaders.end())
        {
            _logger.log(LOG_WARNING) << "There is no shader named '" << name << "'!";
            return 0;
        }
        else
        {
            return shader->second;
        }
    }

    void ShaderManager::reload(const std::string& name)
    {
        if (Shader program_id = ShaderManager::get(name))
        {
            _shaders.erase(name);
            _locations.erase(program_id);

            glDeleteProgram(program_id);

            ShaderManager::create_program(name, _sources.at(program_id));
        }
    }

    void ShaderManager::bind(Shader program_id)
    {
        assert(program_id > 0);
        _apply_properties(program_id);

        glUseProgram(program_id);
        _active_id = program_id;
    }

    void ShaderManager::bind(const std::string& name)
    {
        Shader program_id = ShaderManager::get(name);
        ShaderManager::bind(program_id);
    }

    void ShaderManager::unbind()
    {
        glUseProgram(0);
        _active_id = 0;
    }

    void ShaderManager::set_float(const std::string& name, float value)
    {
        glUniform1f(_location(name), value);
    }

    void ShaderManager::set_float2(const std::string& name, glm::vec2 value)
    {
        glUniform2f(_location(name), value.x, value.y);
    }

    void ShaderManager::set_float3(const std::string& name, glm::vec3 value)
    {
        glUniform3f(_location(name), value.x, value.y, value.z);
    }

    void ShaderManager::set_float4(const std::string& name, glm::vec4 value)
    {
        glUniform4f(_location(name), value.x, value.y, value.z, value.w);
    }

    void ShaderManager::set_float_array(const std::string& name, float* value, int32_t count)
    {
        glUniform1fv(_location(name), count, value);
    }

#   if !defined(APORIA_EMSCRIPTEN)
    void ShaderManager::set_double(const std::string& name, double value)
    {
        glUniform1d(_location(name), value);
    }

    void ShaderManager::set_double2(const std::string& name, glm::dvec2 value)
    {
        glUniform2d(_location(name), value.x, value.y);
    }

    void ShaderManager::set_double3(const std::string& name, glm::dvec3 value)
    {
        glUniform3d(_location(name), value.x, value.y, value.z);
    }

    void ShaderManager::set_double4(const std::string& name, glm::dvec4 value)
    {
        glUniform4d(_location(name), value.x, value.y, value.z, value.w);
    }

    void ShaderManager::set_double_array(const std::string& name, double* value, int32_t count)
    {
        glUniform1dv(_location(name), count, value);
    }
#   endif

    void ShaderManager::set_int(const std::string& name, int32_t value)
    {
        glUniform1i(_location(name), value);
    }

    void ShaderManager::set_int2(const std::string& name, glm::ivec2 value)
    {
        glUniform2i(_location(name), value.x, value.y);
    }

    void ShaderManager::set_int3(const std::string& name, glm::ivec3 value)
    {
        glUniform3i(_location(name), value.x, value.y, value.z);
    }

    void ShaderManager::set_int4(const std::string& name, glm::ivec4 value)
    {
        glUniform4i(_location(name), value.x, value.y, value.z, value.w);
    }

    void ShaderManager::set_int_array(const std::string& name, int* value, int32_t count)
    {
        glUniform1iv(_location(name), count, value);
    }

    void ShaderManager::set_uint(const std::string& name, uint32_t value)
    {
        glUniform1ui(_location(name), value);
    }

    void ShaderManager::set_uint2(const std::string& name, glm::uvec2 value)
    {
        glUniform2ui(_location(name), value.x, value.y);
    }

    void ShaderManager::set_uint3(const std::string& name, glm::uvec3 value)
    {
        glUniform3ui(_location(name), value.x, value.y, value.z);
    }

    void ShaderManager::set_uint4(const std::string& name, glm::uvec4 value)
    {
        glUniform4ui(_location(name), value.x, value.y, value.z, value.w);
    }

    void ShaderManager::set_uint_array(const std::string& name, uint32_t* value, int32_t count)
    {
        glUniform1uiv(_location(name), count, value);
    }

    void ShaderManager::set_mat2(const std::string& name, glm::mat2 value, bool transpose /* = false */, int32_t count /* = 1 */)
    {
        glUniformMatrix2fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void ShaderManager::set_mat3(const std::string& name, glm::mat3 value, bool transpose /* = false */, int32_t count /* = 1 */)
    {
        glUniformMatrix3fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void ShaderManager::set_mat4(const std::string& name, glm::mat4 value, bool transpose /* = false */, int32_t count /* = 1 */)
    {
        glUniformMatrix4fv(_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    Shader ShaderManager::_load(const std::string& contents, ShaderType type)
    {
        const uint32_t shader_type = to_opengl_type(type);

        Shader shader_id = glCreateShader(shader_type);
        const char* shader = contents.c_str();

        glShaderSource(shader_id, 1, &shader, nullptr);
        glCompileShader(shader_id);

        int32_t results;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &results);
        if (results == GL_FALSE)
        {
            int32_t length;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

            std::vector<GLchar> msg(length);
            glGetShaderInfoLog(shader_id, length, &length, &msg[0]);

            glDeleteProgram(shader_id);

            _logger.log(LOG_ERROR) << std::string{ &msg[0] };

            return 0;
        }

        return shader_id;
    }

    void ShaderManager::_link(Shader program_id, const std::vector<ShaderRef>& loaded_shaders)
    {
        for (Shader shader_id : loaded_shaders)
        {
            glAttachShader(program_id, shader_id);
        }

        glLinkProgram(program_id);
        glValidateProgram(program_id);

        for (Shader shader_id : loaded_shaders)
        {
            glDetachShader(program_id, shader_id);
            glDeleteShader(shader_id);
        }
    }

    void ShaderManager::_default_invalids(ShaderProperties& properties)
    {
        if (properties.blend[0] == ShaderBlend::Default)
        {
            properties.blend[0] = _config.default_properties.blend[0];
            properties.blend[1] = _config.default_properties.blend[1];
        }

        if (properties.blend_op == ShaderBlendOp::Default)
        {
            properties.blend_op = _config.default_properties.blend_op;
        }

        if (properties.depth_test == ShaderDepthTest::Default)
        {
            properties.depth_test = _config.default_properties.depth_test;
        }

        if (properties.depth_write == ShaderDepthWrite::Default)
        {
            properties.depth_write = _config.default_properties.depth_write;
        }
    }

    void ShaderManager::_apply_properties(Shader program_id)
    {
        const ShaderProperties& properties = _properties.at(program_id);

        if (properties.blend[0] != ShaderBlend::Off)
        {
            glEnable(GL_BLEND);
            glBlendFunc(to_opengl_type(properties.blend[0]), to_opengl_type(properties.blend[1]));
            glBlendEquation(to_opengl_type(properties.blend_op));
        }
        else
        {
            glDisable(GL_BLEND);
        }

        if (properties.depth_test != ShaderDepthTest::Off)
        {
            glEnable(GL_DEPTH_TEST);

            glDepthFunc(to_opengl_type(properties.depth_test));
            glDepthMask(to_opengl_type(properties.depth_write));
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }

    int32_t ShaderManager::_location(const std::string& name)
    {
        int32_t location;

        std::unordered_map<std::string, int32_t>& locations_dict = _locations[_active_id];
        if (!locations_dict.contains(name))
        {
            location = glGetUniformLocation(_active_id, name.c_str());
            if (location == -1)
            {
                _logger.log(LOG_ERROR) << "'" << name << "' does not correspond to an active uniform variable in program " << _active_id << "!";
            }
            else
            {
                locations_dict.emplace(name, location);
            }
        }
        else
        {
            location = locations_dict.at(name);
        }

        return location;
    }
}
