#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

#include "logger.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    class ShaderManager final
    {
    public:
        ShaderManager(Logger& logger);
        ~ShaderManager();

        Shader create_program(std::string name, const std::string& path);
        Shader get(const std::string& name) const;

        void bind(Shader shader_id);
        void bind(const std::string& name);
        void unbind();

    private:
        Shader _load(const std::string& contents, ShaderType type);
        void _link(Shader shader_id, const std::vector<ShaderRef>& loaded_shaders);

    public:
        void set_float(const std::string& name, float value);
        void set_float2(const std::string& name, glm::vec2 value);
        void set_float3(const std::string& name, glm::vec3 value);
        void set_float4(const std::string& name, glm::vec4 value);
        void set_float_array(const std::string& name, float* value, int32_t count);

#       if !defined(APORIA_EMSCRIPTEN)
        void set_double(const std::string& name, double value);
        void set_double2(const std::string& name, glm::dvec2 value);
        void set_double3(const std::string& name, glm::dvec3 value);
        void set_double4(const std::string& name, glm::dvec4 value);
        void set_double_array(const std::string& name, double* value, int32_t count);
#       endif

        void set_int(const std::string& name, int32_t value);
        void set_int2(const std::string& name, glm::ivec2 value);
        void set_int3(const std::string& name, glm::ivec3 value);
        void set_int4(const std::string& name, glm::ivec4 value);
        void set_int_array(const std::string& name, int* value, int32_t count);

        void set_uint(const std::string& name, uint32_t value);
        void set_uint2(const std::string& name, glm::uvec2 value);
        void set_uint3(const std::string& name, glm::uvec3 value);
        void set_uint4(const std::string& name, glm::uvec4 value);
        void set_uint_array(const std::string& name, uint32_t* value, int32_t count);

        void set_mat2(const std::string& name, glm::mat2 value, bool transpose = false, int32_t count = 1);
        void set_mat3(const std::string& name, glm::mat3 value, bool transpose = false, int32_t count = 1);
        void set_mat4(const std::string& name, glm::mat4 value, bool transpose = false, int32_t count = 1);

    private:
        int32_t _location(const std::string& name);

    private:
        Logger& _logger;

        std::unordered_map<std::string, ShaderRef> _shaders;
        std::map<ShaderRef, std::unordered_map<std::string, int32_t>> _locations;

        ShaderRef _active_id = 0;
    };
}
