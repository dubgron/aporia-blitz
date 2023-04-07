#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "aporia_types.hpp"

namespace Aporia
{
    using ShaderID = u32;

    enum class ShaderType : u8
    {
        Invalid,
        Fragment,
        Vertex
    };
    ShaderType string_to_shader_type(std::string_view type);
    u32 to_opengl_type(ShaderType type);

    enum class ShaderBlend : u8
    {
        Default,
        Off,
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };
    ShaderBlend string_to_shader_blend(std::string_view blend);
    u32 to_opengl_type(ShaderBlend blend);

    enum class ShaderBlendOp : u8
    {
        Default,
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };
    ShaderBlendOp string_to_shader_blend_op(std::string_view blend_op);
    u32 to_opengl_type(ShaderBlendOp blend_op);

    enum class ShaderDepthTest : u8
    {
        Default,
        Off,
        Always,
        Never,
        Less,
        LEqual,
        Greater,
        GEqual,
        Equal,
        NotEqual
    };
    ShaderDepthTest string_to_shader_depth_test(std::string_view depth_test);
    u32 to_opengl_type(ShaderDepthTest depth_test);

    enum class ShaderDepthWrite : u8
    {
        Default,
        On,
        Off
    };
    ShaderDepthWrite string_to_shader_depth_write(std::string_view depth_write);
    u32 to_opengl_type(ShaderDepthWrite depth_write);

    struct ShaderData
    {
        ShaderType type{ ShaderType::Invalid };
        std::string contents;
    };

    struct ShaderProperties
    {
        ShaderBlend blend[2]{ ShaderBlend::Default };
        ShaderBlendOp blend_op{ ShaderBlendOp::Default };
        ShaderDepthTest depth_test{ ShaderDepthTest::Default };
        ShaderDepthWrite depth_write{ ShaderDepthWrite::Default };
    };

    struct ShaderProgramData
    {
        std::vector<ShaderData> shaders;
        ShaderProperties properties;
    };

    struct ShaderInfo
    {
        std::string source;
        ShaderProperties properties;
        std::unordered_map<std::string, i32> locations;
    };

    ShaderID create_shader(std::string_view filepath);
    void remove_shader(ShaderID program_id);
    void remove_all_shaders();

    void set_default_shader_properties(ShaderProperties shader_properties);

    void reload_shader(ShaderID program_id);

    void bind_shader(ShaderID program_id);
    void unbind_shader();

    void shader_set_float(const std::string& name, f32 value);
    void shader_set_float2(const std::string& name, v2 value);
    void shader_set_float3(const std::string& name, v3 value);
    void shader_set_float4(const std::string& name, v4 value);
    void shader_set_float_array(const std::string& name, f32* value, i32 count);

#if !defined(APORIA_EMSCRIPTEN)
    void shader_set_double(const std::string& name, f64 value);
    void shader_set_double2(const std::string& name, v2_f64 value);
    void shader_set_double3(const std::string& name, v3_f64 value);
    void shader_set_double4(const std::string& name, v4_f64 value);
    void shader_set_double_array(const std::string& name, f64* value, i32 count);
#endif

    void shader_set_int(const std::string& name, i32 value);
    void shader_set_int2(const std::string& name, v2_i32 value);
    void shader_set_int3(const std::string& name, v3_i32 value);
    void shader_set_int4(const std::string& name, v4_i32 value);
    void shader_set_int_array(const std::string& name, i32* value, i32 count);

    void shader_set_uint(const std::string& name, u32 value);
    void shader_set_uint2(const std::string& name, v2_u32 value);
    void shader_set_uint3(const std::string& name, v3_u32 value);
    void shader_set_uint4(const std::string& name, v4_u32 value);
    void shader_set_uint_array(const std::string& name, u32* value, i32 count);

    void shader_set_mat2(const std::string& name, m2 value, bool transpose = false, i32 count = 1);
    void shader_set_mat3(const std::string& name, m3 value, bool transpose = false, i32 count = 1);
    void shader_set_mat4(const std::string& name, m4 value, bool transpose = false, i32 count = 1);
}
