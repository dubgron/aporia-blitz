#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "graphics/opengl.hpp"

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

    constexpr inline uint32_t to_opengl_type(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Fragment:  return GL_FRAGMENT_SHADER;
        case ShaderType::Vertex:    return GL_VERTEX_SHADER;
        default:                    assert("Invalid ShaderType" && 0); return 0;
        }
    }

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

    constexpr inline uint32_t to_opengl_type(ShaderBlend blend)
    {
        switch (blend)
        {
        case ShaderBlend::Zero:                     return GL_ZERO;
        case ShaderBlend::One:                      return GL_ONE;
        case ShaderBlend::SrcColor:                 return GL_SRC_COLOR;
        case ShaderBlend::OneMinusSrcColor:         return GL_ONE_MINUS_SRC_COLOR;
        case ShaderBlend::DstColor:                 return GL_DST_COLOR;
        case ShaderBlend::OneMinusDstColor:         return GL_ONE_MINUS_DST_COLOR;
        case ShaderBlend::SrcAlpha:                 return GL_SRC_ALPHA;
        case ShaderBlend::OneMinusSrcAlpha:         return GL_ONE_MINUS_SRC_ALPHA;
        case ShaderBlend::DstAlpha:                 return GL_DST_ALPHA;
        case ShaderBlend::OneMinusDstAlpha:         return GL_ONE_MINUS_DST_ALPHA;
        case ShaderBlend::ConstantColor:            return GL_CONSTANT_COLOR;
        case ShaderBlend::OneMinusConstantColor:    return GL_ONE_MINUS_CONSTANT_COLOR;
        case ShaderBlend::ConstantAlpha:            return GL_CONSTANT_ALPHA;
        case ShaderBlend::OneMinusConstantAlpha:    return GL_ONE_MINUS_CONSTANT_ALPHA;
        case ShaderBlend::SrcAlphaSaturate:         return GL_SRC_ALPHA_SATURATE;
        case ShaderBlend::Src1Color:                return GL_SRC1_COLOR;
        case ShaderBlend::OneMinusSrc1Color:        return GL_ONE_MINUS_SRC1_COLOR;
        case ShaderBlend::Src1Alpha:                return GL_SRC1_ALPHA;
        case ShaderBlend::OneMinusSrc1Alpha:        return GL_ONE_MINUS_SRC1_ALPHA;
        default:                                    assert("Invalid ShaderBlend" && 0); return 0;
        }
    }

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
        if (blend_op == "add")          return ShaderBlendOp::Add;
        else if (blend_op == "sub")     return ShaderBlendOp::Subtract;
        else if (blend_op == "rev_sub") return ShaderBlendOp::ReverseSubtract;
        else if (blend_op == "min")     return ShaderBlendOp::Min;
        else if (blend_op == "max")     return ShaderBlendOp::Max;
        else                            assert("Invalid ShaderBlendOp" && 0); return ShaderBlendOp::Default;
    };

    constexpr inline uint32_t to_opengl_type(ShaderBlendOp blend_op)
    {
        switch (blend_op)
        {
        case ShaderBlendOp::Add:                return GL_FUNC_ADD;
        case ShaderBlendOp::Subtract:           return GL_FUNC_SUBTRACT;
        case ShaderBlendOp::ReverseSubtract:    return GL_FUNC_REVERSE_SUBTRACT;
        case ShaderBlendOp::Min:                return GL_MIN;
        case ShaderBlendOp::Max:                return GL_MAX;
        default:                                assert("Invalid ShaderBlendOp" && 0); return 0;
        }
    }

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

    constexpr inline uint32_t to_opengl_type(ShaderDepthTest depth_test)
    {
        switch (depth_test)
        {
        case ShaderDepthTest::Always:   return GL_ALWAYS;
        case ShaderDepthTest::Never:    return GL_NEVER;
        case ShaderDepthTest::Less:     return GL_LESS;
        case ShaderDepthTest::LEqual:   return GL_LEQUAL;
        case ShaderDepthTest::Greater:  return GL_GREATER;
        case ShaderDepthTest::GEqual:   return GL_GEQUAL;
        case ShaderDepthTest::Equal:    return GL_EQUAL;
        case ShaderDepthTest::NotEqual: return GL_NOTEQUAL;
        default:                        assert("Invalid ShaderDepthTest" && 0); return 0;
        }
    }

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

    constexpr inline uint32_t to_opengl_type(ShaderDepthWrite depth_write)
    {
        switch (depth_write)
        {
        case ShaderDepthWrite::On:  return GL_TRUE;
        case ShaderDepthWrite::Off: return GL_FALSE;
        default:                    assert("Invalid ShaderDepthWrite" && 0); return 0;
        }
    }

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
