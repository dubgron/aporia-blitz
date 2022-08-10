#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "platform/opengl.hpp"

namespace Aporia
{
    template<typename T>
    constexpr inline T string_to(std::string_view string);

    enum class ShaderType : uint8_t
    {
        Invalid,
        Fragment,
        Vertex
    };

    template<>
    constexpr inline ShaderType string_to(std::string_view type)
    {
        if (type == "fragment")     return ShaderType::Fragment;
        else if (type == "vertex")  return ShaderType::Vertex;
        else                        assert("Invalid ShaderType" && 0); return ShaderType::Invalid;
    };

    uint32_t to_opengl_type(ShaderType type);

    enum class ShaderBlend : uint8_t
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
    constexpr inline ShaderBlend string_to(std::string_view blend)
    {
        if (blend == "off")                             return ShaderBlend::Off;
        else if (blend == "zero")                       return ShaderBlend::Zero;
        else if (blend == "one")                        return ShaderBlend::One;
        else if (blend == "src_color")                  return ShaderBlend::SrcColor;
        else if (blend == "one_minus_src_color")        return ShaderBlend::OneMinusSrcColor;
        else if (blend == "dst_color")                  return ShaderBlend::DstColor;
        else if (blend == "one_minus_dst_color")        return ShaderBlend::OneMinusDstColor;
        else if (blend == "src_alpha")                  return ShaderBlend::SrcAlpha;
        else if (blend == "one_minus_src_alpha")        return ShaderBlend::OneMinusSrcAlpha;
        else if (blend == "dst_alpha")                  return ShaderBlend::DstAlpha;
        else if (blend == "one_minus_dst_alpha")        return ShaderBlend::OneMinusDstAlpha;
        else if (blend == "constant_color")             return ShaderBlend::ConstantColor;
        else if (blend == "one_minus_constant_alpha")   return ShaderBlend::OneMinusConstantColor;
        else if (blend == "constant_alpha")             return ShaderBlend::ConstantAlpha;
        else if (blend == "one_minus_constant_alpha")   return ShaderBlend::OneMinusConstantAlpha;
        else if (blend == "src_alpha_saturate")         return ShaderBlend::SrcAlphaSaturate;
        else if (blend == "src_1_color")                return ShaderBlend::Src1Color;
        else if (blend == "one_minus_src_1_color")      return ShaderBlend::OneMinusSrc1Color;
        else if (blend == "src_1_alpha")                return ShaderBlend::Src1Alpha;
        else if (blend == "one_minus_src_1_alpha")      return ShaderBlend::OneMinusSrc1Alpha;
        else                                            assert("Invalid ShaderBlend" && 0); return ShaderBlend::Default;
    };

    uint32_t to_opengl_type(ShaderBlend blend);

    enum class ShaderBlendOp : uint8_t
    {
        Default,
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    template<>
    constexpr inline ShaderBlendOp string_to(std::string_view blend_op)
    {
        if (blend_op == "add")              return ShaderBlendOp::Add;
        else if (blend_op == "sub")         return ShaderBlendOp::Subtract;
        else if (blend_op == "rev_sub")     return ShaderBlendOp::ReverseSubtract;
        else if (blend_op == "min")         return ShaderBlendOp::Min;
        else if (blend_op == "max")         return ShaderBlendOp::Max;
        else                                assert("Invalid ShaderBlendOp" && 0); return ShaderBlendOp::Default;
    };

    uint32_t to_opengl_type(ShaderBlendOp blend_op);

    enum class ShaderDepthTest : uint8_t
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
    constexpr inline ShaderDepthTest string_to(std::string_view depth_test)
    {
        if (depth_test == "off")            return ShaderDepthTest::Off;
        else if (depth_test == "always")    return ShaderDepthTest::Always;
        else if (depth_test == "never")     return ShaderDepthTest::Never;
        else if (depth_test == "less")      return ShaderDepthTest::Less;
        else if (depth_test == "lequal")    return ShaderDepthTest::LEqual;
        else if (depth_test == "greater")   return ShaderDepthTest::Greater;
        else if (depth_test == "gequal")    return ShaderDepthTest::GEqual;
        else if (depth_test == "equal")     return ShaderDepthTest::Equal;
        else if (depth_test == "notequal")  return ShaderDepthTest::NotEqual;
        else                                assert("Invalid ShaderDepthTest" && 0); return ShaderDepthTest::Default;
    };

    uint32_t to_opengl_type(ShaderDepthTest depth_test);

    enum class ShaderDepthWrite : uint8_t
    {
        Default,
        On,
        Off
    };

    template<>
    constexpr inline ShaderDepthWrite string_to(std::string_view depth_write)
    {
        if (depth_write == "on")        return ShaderDepthWrite::On;
        else if (depth_write == "off")  return ShaderDepthWrite::Off;
        else                            assert("Invalid ShaderDepthWrite" && 0); return ShaderDepthWrite::Default;
    };

    uint32_t to_opengl_type(ShaderDepthWrite depth_write);

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
