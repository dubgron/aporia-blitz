#include "shader_properties.hpp"

namespace Aporia
{
    u32 to_opengl_type(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Fragment:  return GL_FRAGMENT_SHADER;
        case ShaderType::Vertex:    return GL_VERTEX_SHADER;
        default:                    assert("Invalid ShaderType" && 0); return 0;
        }
    }

    u32 to_opengl_type(ShaderBlend blend)
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

    u32 to_opengl_type(ShaderBlendOp blend_op)
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

    u32 to_opengl_type(ShaderDepthTest depth_test)
    {
        switch (depth_test)
        {
        case ShaderDepthTest::Always:       return GL_ALWAYS;
        case ShaderDepthTest::Never:        return GL_NEVER;
        case ShaderDepthTest::Less:         return GL_LESS;
        case ShaderDepthTest::LEqual:       return GL_LEQUAL;
        case ShaderDepthTest::Greater:      return GL_GREATER;
        case ShaderDepthTest::GEqual:       return GL_GEQUAL;
        case ShaderDepthTest::Equal:        return GL_EQUAL;
        case ShaderDepthTest::NotEqual:     return GL_NOTEQUAL;
        default:                            assert("Invalid ShaderDepthTest" && 0); return 0;
        }
    }

    u32 to_opengl_type(ShaderDepthWrite depth_write)
    {
        switch (depth_write)
        {
        case ShaderDepthWrite::On:      return GL_TRUE;
        case ShaderDepthWrite::Off:     return GL_FALSE;
        default:                        assert("Invalid ShaderDepthWrite" && 0); return 0;
        }
    }
}
