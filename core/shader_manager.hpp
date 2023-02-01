#pragma once

#include <cassert>
#include <map>
#include <string>
#include <unordered_map>

#include "aporia_types.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_properties.hpp"

namespace Aporia
{
    struct ShaderConfig;

    class ShaderManager final
    {
    public:
        ShaderManager(const ShaderConfig& config);
        ~ShaderManager();

        Shader create_program(const std::string& name, const std::string& filepath);
        Shader get(const std::string& name) const;

        void reload(const std::string& name);

        void bind(Shader program_id);
        void bind(const std::string& name);
        void unbind();

        void set_float(const std::string& name, f32 value);
        void set_float2(const std::string& name, v2 value);
        void set_float3(const std::string& name, v3 value);
        void set_float4(const std::string& name, v4 value);
        void set_float_array(const std::string& name, f32* value, i32 count);

#       if !defined(APORIA_EMSCRIPTEN)
        void set_double(const std::string& name, f64 value);
        void set_double2(const std::string& name, v2_f64 value);
        void set_double3(const std::string& name, v3_f64 value);
        void set_double4(const std::string& name, v4_f64 value);
        void set_double_array(const std::string& name, f64* value, i32 count);
#       endif

        void set_int(const std::string& name, i32 value);
        void set_int2(const std::string& name, v2_i32 value);
        void set_int3(const std::string& name, v3_i32 value);
        void set_int4(const std::string& name, v4_i32 value);
        void set_int_array(const std::string& name, i32* value, i32 count);

        void set_uint(const std::string& name, u32 value);
        void set_uint2(const std::string& name, v2_u32 value);
        void set_uint3(const std::string& name, v3_u32 value);
        void set_uint4(const std::string& name, v4_u32 value);
        void set_uint_array(const std::string& name, u32* value, i32 count);

        void set_mat2(const std::string& name, m2 value, bool transpose = false, i32 count = 1);
        void set_mat3(const std::string& name, m3 value, bool transpose = false, i32 count = 1);
        void set_mat4(const std::string& name, m4 value, bool transpose = false, i32 count = 1);

    private:
        Shader _load(const std::string& contents, ShaderType type);
        void _link(Shader program_id, const std::vector<ShaderRef>& loaded_shaders);

        void _default_invalids(ShaderProperties& properties);
        void _apply_properties(Shader program_id);

        i32 _location(const std::string& name);

    private:
        const ShaderConfig& _config;

        std::unordered_map<std::string, ShaderRef> _shaders;
        std::map<ShaderRef, std::unordered_map<std::string, i32>> _locations;
        std::map<ShaderRef, std::string> _sources;
        std::map<ShaderRef, ShaderProperties> _properties;

        ShaderRef _active_id = 0;
    };
}
