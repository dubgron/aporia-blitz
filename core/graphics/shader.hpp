#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "logger.hpp"

namespace Aporia
{
    class Shader final
    {
    public:
        enum class Type
        {
            Fragment,
            Vertex
        };

        Shader(Logger& logger);

        void bind() const;
        void unbind() const;

        bool load_shader(std::string path, Type type);
        void compile();

        void set_float(const std::string& name, float value);
        void set_float2(const std::string& name, glm::vec2 value);
        void set_float3(const std::string& name, glm::vec3 value);
        void set_float4(const std::string& name, glm::vec4 value);
        void set_float_array(const std::string& name, float* value, int32_t count);

        void set_double(const std::string& name, double value);
        void set_double2(const std::string& name, glm::dvec2 value);
        void set_double3(const std::string& name, glm::dvec3 value);
        void set_double4(const std::string& name, glm::dvec4 value);
        void set_double_array(const std::string& name, double* value, int32_t count);

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

        Logger& _logger;

        uint32_t _program = 0;
        std::unordered_map<std::string, int32_t> _locations;
        std::vector<uint32_t> _shaders;
    };
}
