#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "aporia_types.hpp"

namespace Aporia
{
    template<typename T>
    constexpr T string_to(std::string_view string);

    enum class ShaderType : u8
    {
        Invalid,
        Fragment,
        Vertex
    };

    template<>
    constexpr ShaderType string_to(std::string_view type)
    {
        if (type == "fragment")     return ShaderType::Fragment;
        if (type == "vertex")       return ShaderType::Vertex;

        assert("Invalid ShaderType" && 0);
        return ShaderType::Invalid;
    }

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

    template<>
    constexpr ShaderBlend string_to(std::string_view blend)
    {
        if (blend == "off")                        return ShaderBlend::Off;
        if (blend == "zero")                       return ShaderBlend::Zero;
        if (blend == "one")                        return ShaderBlend::One;
        if (blend == "src_color")                  return ShaderBlend::SrcColor;
        if (blend == "one_minus_src_color")        return ShaderBlend::OneMinusSrcColor;
        if (blend == "dst_color")                  return ShaderBlend::DstColor;
        if (blend == "one_minus_dst_color")        return ShaderBlend::OneMinusDstColor;
        if (blend == "src_alpha")                  return ShaderBlend::SrcAlpha;
        if (blend == "one_minus_src_alpha")        return ShaderBlend::OneMinusSrcAlpha;
        if (blend == "dst_alpha")                  return ShaderBlend::DstAlpha;
        if (blend == "one_minus_dst_alpha")        return ShaderBlend::OneMinusDstAlpha;
        if (blend == "constant_color")             return ShaderBlend::ConstantColor;
        if (blend == "one_minus_constant_alpha")   return ShaderBlend::OneMinusConstantColor;
        if (blend == "constant_alpha")             return ShaderBlend::ConstantAlpha;
        if (blend == "one_minus_constant_alpha")   return ShaderBlend::OneMinusConstantAlpha;
        if (blend == "src_alpha_saturate")         return ShaderBlend::SrcAlphaSaturate;
        if (blend == "src_1_color")                return ShaderBlend::Src1Color;
        if (blend == "one_minus_src_1_color")      return ShaderBlend::OneMinusSrc1Color;
        if (blend == "src_1_alpha")                return ShaderBlend::Src1Alpha;
        if (blend == "one_minus_src_1_alpha")      return ShaderBlend::OneMinusSrc1Alpha;

        assert("Invalid ShaderBlend" && 0);
        return ShaderBlend::Default;
    }

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

    template<>
    constexpr ShaderBlendOp string_to(std::string_view blend_op)
    {
        if (blend_op == "add")      return ShaderBlendOp::Add;
        if (blend_op == "sub")      return ShaderBlendOp::Subtract;
        if (blend_op == "rev_sub")  return ShaderBlendOp::ReverseSubtract;
        if (blend_op == "min")      return ShaderBlendOp::Min;
        if (blend_op == "max")      return ShaderBlendOp::Max;

        assert("Invalid ShaderBlendOp" && 0);
        return ShaderBlendOp::Default;
    }

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

    template<>
    constexpr ShaderDepthTest string_to(std::string_view depth_test)
    {
        if (depth_test == "off")       return ShaderDepthTest::Off;
        if (depth_test == "always")    return ShaderDepthTest::Always;
        if (depth_test == "never")     return ShaderDepthTest::Never;
        if (depth_test == "less")      return ShaderDepthTest::Less;
        if (depth_test == "lequal")    return ShaderDepthTest::LEqual;
        if (depth_test == "greater")   return ShaderDepthTest::Greater;
        if (depth_test == "gequal")    return ShaderDepthTest::GEqual;
        if (depth_test == "equal")     return ShaderDepthTest::Equal;
        if (depth_test == "notequal")  return ShaderDepthTest::NotEqual;

        assert("Invalid ShaderDepthTest" && 0);
        return ShaderDepthTest::Default;
    }

    u32 to_opengl_type(ShaderDepthTest depth_test);

    enum class ShaderDepthWrite : u8
    {
        Default,
        On,
        Off
    };

    template<>
    constexpr ShaderDepthWrite string_to(std::string_view depth_write)
    {
        if (depth_write == "on")   return ShaderDepthWrite::On;
        if (depth_write == "off")  return ShaderDepthWrite::Off;

        assert("Invalid ShaderDepthWrite" && 0);
        return ShaderDepthWrite::Default;
    }

    u32 to_opengl_type(ShaderDepthWrite depth_write);

    struct SubShaderData final
    {
        ShaderType type{ ShaderType::Invalid };
        std::string contents;
    };

    struct ShaderProperties final
    {
        ShaderBlend blend[2]{ ShaderBlend::Default };
        ShaderBlendOp blend_op{ ShaderBlendOp::Default };
        ShaderDepthTest depth_test{ ShaderDepthTest::Default };
        ShaderDepthWrite depth_write{ ShaderDepthWrite::Default };
    };

    struct ShaderData final
    {
        ShaderProperties properties;
        std::vector<SubShaderData> subshaders;
    };
}
