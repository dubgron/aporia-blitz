#include "aporia_shaders.hpp"

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"
#include "platform/aporia_opengl.hpp"

namespace Aporia
{
    u32 default_shader = 0;
    u32 line_shader = 0;
    u32 circle_shader = 0;
    u32 font_shader = 0;
    u32 postprocessing_shader = 0;
    u32 raymarching_shader = 0;
    u32 shadowcasting_shader = 0;

    u32 editor_grid_shader = 0;

    static std::unordered_map<u32, ShaderInfo> shaders;
    static u32 active_shader_id = 0;
    static ShaderProperties default_shader_properties;

    SubShaderType string_to_subshader_type(String type)
    {
        if (type == "fragment")     return SubShaderType::Fragment;
        if (type == "vertex")       return SubShaderType::Vertex;

        APORIA_LOG(Critical, "Wrong Shader Type! Expected 'fragment' or 'vertex'. Got '{}', len: {}.", type, type.length);
        APORIA_UNREACHABLE();
        return SubShaderType::Invalid;
    }

    u32 to_opengl_type(SubShaderType type)
    {
        switch (type)
        {
            case SubShaderType::Fragment:   return GL_FRAGMENT_SHADER;
            case SubShaderType::Vertex:     return GL_VERTEX_SHADER;
            default:                        APORIA_UNREACHABLE(); return 0;
        }
    }

    ShaderBlend string_to_shader_blend(String blend)
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

        APORIA_LOG(Critical, "Wrong Shader Blend! Expected 'off', 'zero', 'one', 'src_color', 'one_minus_src_color', 'dst_color', 'one_minus_dst_color', 'src_alpha', 'one_minus_src_alpha', 'dst_alpha', 'one_minus_dst_alpha', 'constant_color', 'one_minus_constant_alpha', 'constant_alpha', 'one_minus_constant_alpha', 'src_alpha_saturate', 'src_1_color', 'one_minus_src_1_color', 'src_1_alpha' or 'one_minus_src_1_alpha'. Got '{}', len: {}.", blend, blend.length);
        APORIA_UNREACHABLE();
        return ShaderBlend::Default;
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
#if !defined(APORIA_EMSCRIPTEN)
            case ShaderBlend::Src1Color:                return GL_SRC1_COLOR;
            case ShaderBlend::OneMinusSrc1Color:        return GL_ONE_MINUS_SRC1_COLOR;
            case ShaderBlend::Src1Alpha:                return GL_SRC1_ALPHA;
            case ShaderBlend::OneMinusSrc1Alpha:        return GL_ONE_MINUS_SRC1_ALPHA;
#endif
            default:                                    APORIA_UNREACHABLE(); return 0;
        }
    }

    ShaderBlendOp string_to_shader_blend_op(String blend_op)
    {
        if (blend_op == "add")      return ShaderBlendOp::Add;
        if (blend_op == "sub")      return ShaderBlendOp::Subtract;
        if (blend_op == "rev_sub")  return ShaderBlendOp::ReverseSubtract;
        if (blend_op == "min")      return ShaderBlendOp::Min;
        if (blend_op == "max")      return ShaderBlendOp::Max;

        APORIA_LOG(Critical, "Wrong Shader Blend Operation! Expected 'add', 'sub', 'rev_sub', 'min' or 'max'. Got '{}', len: {}.", blend_op, blend_op.length);
        APORIA_UNREACHABLE();
        return ShaderBlendOp::Default;
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
            default:                                APORIA_UNREACHABLE(); return 0;
        }
    }

    ShaderDepthTest string_to_shader_depth_test(String depth_test)
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

        APORIA_LOG(Critical, "Wrong Shader Blend Operation! Expected 'off', 'always', 'never', 'less', 'lequal', 'greater', 'gequal', 'equal' or 'notequal'. Got '{}', len: {}.", depth_test, depth_test.length);
        APORIA_UNREACHABLE();
        return ShaderDepthTest::Default;
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
            default:                            APORIA_UNREACHABLE(); return 0;
        }
    }

    ShaderDepthWrite string_to_shader_depth_write(String depth_write)
    {
        if (depth_write == "on")   return ShaderDepthWrite::On;
        if (depth_write == "off")  return ShaderDepthWrite::Off;

        APORIA_LOG(Critical, "Wrong Shader Blend Operation! Expected 'on' or 'off'. Got '{}', len: {}.", depth_write, depth_write.length);
        APORIA_UNREACHABLE();
        return ShaderDepthWrite::Default;
    }

    u32 to_opengl_type(ShaderDepthWrite depth_write)
    {
        switch (depth_write)
        {
            case ShaderDepthWrite::On:      return GL_TRUE;
            case ShaderDepthWrite::Off:     return GL_FALSE;
            default:                        APORIA_UNREACHABLE(); return 0;
        }
    }

    static ShaderData parse_shader(String contents)
    {
        ShaderData result;

        // Most common case - parsing vertex and fragment subshaders
        result.subshaders.reserve(2);

        u64 line_begin = 0;
        u64 line_end = contents.find_eol(line_begin);

        while (line_end != String::INVALID_INDEX)
        {
            const String line = contents.substr(line_begin, line_end - line_begin);

            if (line.length > 0)
            {
                const u64 params_begin = line.find(' ') + 1;
                const String params = line.substr(params_begin, line_end - params_begin);

                if (line.starts_with("#blend "))
                {
                    const u64 delim = params.find(' ');
                    if (delim != String::INVALID_INDEX)
                    {
                        result.properties.blend[0] = string_to_shader_blend(params.substr(0, delim));
                        result.properties.blend[1] = string_to_shader_blend(params.substr(delim + 1, line_end - delim - 1));
                    }
                    else
                    {
                        result.properties.blend[0] = string_to_shader_blend(params);
                    }
                }
                else if (line.starts_with("#blend_op "))
                {
                    result.properties.blend_op = string_to_shader_blend_op(params);
                }
                else if (line.starts_with("#depth_test "))
                {
                    result.properties.depth_test = string_to_shader_depth_test(params);
                }
                else if (line.starts_with("#depth_write "))
                {
                    result.properties.depth_write = string_to_shader_depth_write(params);
                }
                else if (line.starts_with("#type "))
                {
                    const u64 subshader_begin = line_end + 1;
                    const u64 subshader_end = contents.find("#type ", subshader_begin);

                    SubShaderData subshader;
                    subshader.type = string_to_subshader_type(params);
                    subshader.contents = contents.substr(subshader_begin, subshader_end - subshader_begin);
                    result.subshaders.push_back( std::move(subshader) );

                    line_end = subshader_end - 1;
                }
            }

            line_begin = line_end + 1;
            line_end = contents.find_eol(line_begin);
        }

        return result;
    }

    static u32 load_subshader(String contents, SubShaderType type)
    {
        const u32 opengl_type = to_opengl_type(type);

        const char* shader_code = (const char*)contents.data;
        const i32 shader_length = contents.length;

        const u32 subshader_id = glCreateShader(opengl_type);
        glShaderSource(subshader_id, 1, &shader_code, &shader_length);
        glCompileShader(subshader_id);

        i32 results;
        glGetShaderiv(subshader_id, GL_COMPILE_STATUS, &results);
        if (results == GL_FALSE)
        {
            i32 length;
            glGetShaderiv(subshader_id, GL_INFO_LOG_LENGTH, &length);

            std::vector<GLchar> msg(length);
            glGetShaderInfoLog(subshader_id, length, &length, msg.data());

            glDeleteProgram(subshader_id);

            APORIA_LOG(Error, String{ msg.data() });

            return 0;
        }

        return subshader_id;
    }

    static void link_shaders(u32 shader_id, const std::vector<u32>& loaded_subshaders)
    {
        for (const u32 subshader_id : loaded_subshaders)
        {
            glAttachShader(shader_id, subshader_id);
        }

        glLinkProgram(shader_id);
        glValidateProgram(shader_id);

        for (const u32 subshader_id : loaded_subshaders)
        {
            glDetachShader(shader_id, subshader_id);
            glDeleteShader(subshader_id);
        }
    }

    static void apply_shader_defaults(ShaderProperties& properties)
    {
        if (properties.blend[0] == ShaderBlend::Default)
        {
            properties.blend[0] = default_shader_properties.blend[0];
            properties.blend[1] = default_shader_properties.blend[1];
        }

        if (properties.blend_op == ShaderBlendOp::Default)
        {
            properties.blend_op = default_shader_properties.blend_op;
        }

        if (properties.depth_test == ShaderDepthTest::Default)
        {
            properties.depth_test = default_shader_properties.depth_test;
        }

        if (properties.depth_write == ShaderDepthWrite::Default)
        {
            properties.depth_write = default_shader_properties.depth_write;
        }
    }

    static void apply_shader_properties(u32 shader_id)
    {
        APORIA_ASSERT_WITH_MESSAGE(shaders.contains(shader_id),
            "Shader (with ID: {}) is not valid!", shader_id);

        const ShaderInfo& shader_info = shaders.at(shader_id);
        const ShaderProperties& properties = shader_info.properties;

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

    static i32 get_uniform_location(String name)
    {
        APORIA_ASSERT_WITH_MESSAGE(shaders.contains(active_shader_id),
            "No active shader!");

        ShaderInfo& shader_info = shaders.at(active_shader_id);
        std::unordered_map<String, i32>& locations_dict = shader_info.locations;

        i32 location;
        if (!locations_dict.contains(name))
        {
            location = glGetUniformLocation(active_shader_id, *name);
            if (location == -1)
            {
                APORIA_LOG(Error, "'{}' does not correspond to an active uniform variable in shader {}!", name, active_shader_id);
            }
            else
            {
                name = push_string(&persistent_arena, name);
                locations_dict.emplace(name, location);
            }
        }
        else
        {
            location = locations_dict.at(name);
        }

        return location;
    }

    void shaders_init()
    {
        default_shader_properties = shader_config.default_properties;
    }

    u32 create_shader(String filepath)
    {
        u32 shader_id = glCreateProgram();

        std::vector<u32> loaded_subshaders;
        loaded_subshaders.reserve(2);

        const String shader_contents = read_file(&persistent_arena, filepath);
        ShaderData shader_data = parse_shader(shader_contents);
        for (const SubShaderData& data : shader_data.subshaders)
        {
            if (u32 shader_id = load_subshader(data.contents, data.type))
            {
                loaded_subshaders.push_back(shader_id);
            }
        }

        if (loaded_subshaders.empty())
        {
            glDeleteProgram(shader_id);
            return 0;
        }

        link_shaders(shader_id, loaded_subshaders);

        apply_shader_defaults(shader_data.properties);

        ShaderInfo shader_info;
        shader_info.source = filepath;
        shader_info.properties = shader_data.properties;

        shaders.emplace(shader_id, std::move(shader_info));

        return shader_id;
    }

    void remove_shader(u32 shader_id)
    {
        APORIA_ASSERT_WITH_MESSAGE(shaders.contains(shader_id),
            "Shader (with ID: {}) is not valid!", shader_id);

        glDeleteProgram(shader_id);
        shaders.erase(shader_id);
    }

    void remove_all_shaders()
    {
        for (const auto&[shader_id, shader_info] : shaders)
        {
            glDeleteProgram(shader_id);
        }

        shaders.clear();
    }

    void reload_shader(u32 shader_id)
    {
        APORIA_ASSERT_WITH_MESSAGE(shaders.contains(shader_id),
            "Shader (with ID: {}) is not valid!", shader_id);

        const String shader_source = shaders.at(shader_id).source;

        glDeleteProgram(shader_id);
        shaders.erase(shader_id);

        create_shader(shader_source);
    }

    void bind_shader(u32 shader_id)
    {
        APORIA_ASSERT(shader_id > 0);
        apply_shader_properties(shader_id);

        glUseProgram(shader_id);
        active_shader_id = shader_id;
    }

    void unbind_shader()
    {
        glUseProgram(0);
        active_shader_id = 0;
    }

    void shader_set_float(String name, f32 value)
    {
        glUniform1f(get_uniform_location(name), value);
    }

    void shader_set_float2(String name, v2 value)
    {
        glUniform2f(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_float3(String name, v3 value)
    {
        glUniform3f(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_float4(String name, v4 value)
    {
        glUniform4f(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_float_array(String name, f32* value, i32 count)
    {
        glUniform1fv(get_uniform_location(name), count, value);
    }

#if !defined(APORIA_EMSCRIPTEN)
    void shader_set_double(String name, f64 value)
    {
        glUniform1d(get_uniform_location(name), value);
    }

    void shader_set_double2(String name, v2_f64 value)
    {
        glUniform2d(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_double3(String name, v3_f64 value)
    {
        glUniform3d(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_double4(String name, v4_f64 value)
    {
        glUniform4d(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_double_array(String name, f64* value, i32 count)
    {
        glUniform1dv(get_uniform_location(name), count, value);
    }
#endif

    void shader_set_int(String name, i32 value)
    {
        glUniform1i(get_uniform_location(name), value);
    }

    void shader_set_int2(String name, v2_i32 value)
    {
        glUniform2i(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_int3(String name, v3_i32 value)
    {
        glUniform3i(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_int4(String name, v4_i32 value)
    {
        glUniform4i(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_int_array(String name, i32* value, i32 count)
    {
        glUniform1iv(get_uniform_location(name), count, value);
    }

    void shader_set_uint(String name, u32 value)
    {
        glUniform1ui(get_uniform_location(name), value);
    }

    void shader_set_uint2(String name, v2_u32 value)
    {
        glUniform2ui(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_uint3(String name, v3_u32 value)
    {
        glUniform3ui(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_uint4(String name, v4_u32 value)
    {
        glUniform4ui(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_uint_array(String name, u32* value, i32 count)
    {
        glUniform1uiv(get_uniform_location(name), count, value);
    }

    void shader_set_mat2(String name, m2 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix2fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void shader_set_mat3(String name, m3 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix3fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void shader_set_mat4(String name, m4 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix4fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }
}
