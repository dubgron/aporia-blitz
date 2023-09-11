#pragma once

#include "aporia_assets.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    enum class SubShaderType : u8
    {
        Invalid,
        Fragment,
        Vertex,

        Count,
    };
    SubShaderType string_to_subshader_type(String type);
    u32 to_opengl_type(SubShaderType type);

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
        OneMinusSrc1Alpha,
    };
    ShaderBlend string_to_shader_blend(String blend);
    u32 to_opengl_type(ShaderBlend blend);

    enum class ShaderBlendOp : u8
    {
        Default,
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };
    ShaderBlendOp string_to_shader_blend_op(String blend_op);
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
        NotEqual,
    };
    ShaderDepthTest string_to_shader_depth_test(String depth_test);
    u32 to_opengl_type(ShaderDepthTest depth_test);

    enum class ShaderDepthWrite : u8
    {
        Default,
        On,
        Off,
    };
    ShaderDepthWrite string_to_shader_depth_write(String depth_write);
    u32 to_opengl_type(ShaderDepthWrite depth_write);

    struct SubShaderData
    {
        SubShaderType type{ SubShaderType::Invalid };
        String contents;
    };

    struct ShaderProperties
    {
        ShaderBlend blend[2]{ ShaderBlend::Default };
        ShaderBlendOp blend_op{ ShaderBlendOp::Default };
        ShaderDepthTest depth_test{ ShaderDepthTest::Default };
        ShaderDepthWrite depth_write{ ShaderDepthWrite::Default };
    };

    struct ShaderData
    {
        SubShaderData* subshaders;
        u64 subshaders_count = 0;

        ShaderProperties properties;
    };

    struct ShaderInfo
    {
        u32 shader_id = 0;
        u32 subshaders_count = 0;
        ShaderProperties properties;
        String source_file;
    };

    void shaders_init(MemoryArena* arena);

    u32 load_shader(String filepath, u64 subshaders_count = 2);

    bool reload_shader_asset(Asset* shader_asset);
    bool reload_shader(u32 shader_id);

    void remove_shader(u32 shader_id);
    void remove_all_shaders();

    void bind_shader(u32 shader_id);
    void unbind_shader();

    void shader_set_float(String name, f32 value);
    void shader_set_float2(String name, v2 value);
    void shader_set_float3(String name, v3 value);
    void shader_set_float4(String name, v4 value);
    void shader_set_float_array(String name, f32* value, i32 count);

#if !defined(APORIA_EMSCRIPTEN)
    void shader_set_double(String name, f64 value);
    void shader_set_double2(String name, v2_f64 value);
    void shader_set_double3(String name, v3_f64 value);
    void shader_set_double4(String name, v4_f64 value);
    void shader_set_double_array(String name, f64* value, i32 count);
#endif

    void shader_set_int(String name, i32 value);
    void shader_set_int2(String name, v2_i32 value);
    void shader_set_int3(String name, v3_i32 value);
    void shader_set_int4(String name, v4_i32 value);
    void shader_set_int_array(String name, i32* value, i32 count);

    void shader_set_uint(String name, u32 value);
    void shader_set_uint2(String name, v2_u32 value);
    void shader_set_uint3(String name, v3_u32 value);
    void shader_set_uint4(String name, v4_u32 value);
    void shader_set_uint_array(String name, u32* value, i32 count);

    void shader_set_mat2(String name, m2 value, bool transpose = false, i32 count = 1);
    void shader_set_mat3(String name, m3 value, bool transpose = false, i32 count = 1);
    void shader_set_mat4(String name, m4 value, bool transpose = false, i32 count = 1);

    // Predefined shaders
    extern u32 default_shader;
    extern u32 line_shader;
    extern u32 circle_shader;
    extern u32 font_shader;
    extern u32 postprocessing_shader;
    extern u32 raymarching_shader;
    extern u32 shadowcasting_shader;

    extern u32 editor_grid_shader;
}
