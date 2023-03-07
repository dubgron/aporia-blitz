#include "aporia_shaders.hpp"

#include "configs/shader_config.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    static std::unordered_map<ShaderID, ShaderInfo> programs;
    static ShaderID active_program_id = 0;
    static ShaderProperties default_shader_properties;

    ShaderType string_to_shader_type(std::string_view type)
    {
        if (type == "fragment")     return ShaderType::Fragment;
        if (type == "vertex")       return ShaderType::Vertex;

        assert("Invalid ShaderType" && 0);
        return ShaderType::Invalid;
    }

    u32 to_opengl_type(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Fragment:  return GL_FRAGMENT_SHADER;
        case ShaderType::Vertex:    return GL_VERTEX_SHADER;
        default:                    assert("Invalid ShaderType" && 0); return 0;
        }
    }

    ShaderBlend string_to_shader_blend(std::string_view blend)
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

    ShaderBlendOp string_to_shader_blend_op(std::string_view blend_op)
    {
        if (blend_op == "add")      return ShaderBlendOp::Add;
        if (blend_op == "sub")      return ShaderBlendOp::Subtract;
        if (blend_op == "rev_sub")  return ShaderBlendOp::ReverseSubtract;
        if (blend_op == "min")      return ShaderBlendOp::Min;
        if (blend_op == "max")      return ShaderBlendOp::Max;

        assert("Invalid ShaderBlendOp" && 0);
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
        default:                                assert("Invalid ShaderBlendOp" && 0); return 0;
        }
    }

    ShaderDepthTest string_to_shader_depth_test(std::string_view depth_test)
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

    ShaderDepthWrite string_to_shader_depth_write(std::string_view depth_write)
    {
        if (depth_write == "on")   return ShaderDepthWrite::On;
        if (depth_write == "off")  return ShaderDepthWrite::Off;

        assert("Invalid ShaderDepthWrite" && 0);
        return ShaderDepthWrite::Default;
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

    static ShaderProgramData parse_shader(std::string_view contents)
    {
        ShaderProgramData results;

        /* Most common case - parsing vertex and fragment shaders */
        results.shaders.reserve(2);

        const std::string_view type_token = "#type ";
        const std::string_view version_token = "#version ";

        u64 line_begin = 0;
        u64 line_end = contents.find('\n', line_begin);

        while (line_end != std::string::npos)
        {
            const std::string_view line = contents.substr(line_begin, line_end - line_begin);

            const u64 params_begin = line.find(' ') + 1;
            const std::string_view params = line.substr(params_begin, line_end - params_begin);
            if (line.starts_with("#blend "))
            {
                const u64 delim = params.find(' ');
                if (delim != std::string::npos)
                {
                    results.properties.blend[0] = string_to_shader_blend(params.substr(0, delim));
                    results.properties.blend[1] = string_to_shader_blend(params.substr(delim + 1, line_end - delim - 1));
                }
                else
                {
                    results.properties.blend[0] = string_to_shader_blend(params);
                }
            }
            else if (line.starts_with("#blend_op "))
            {
                results.properties.blend_op = string_to_shader_blend_op(params);
            }
            else if (line.starts_with("#depth_test "))
            {
                results.properties.depth_test = string_to_shader_depth_test(params);
            }
            else if (line.starts_with("#depth_write "))
            {
                results.properties.depth_write = string_to_shader_depth_write(params);
            }
            else if (line.starts_with("#type "))
            {
                const u64 shader_begin = line_end + 1;
                const u64 shader_end = contents.find("#type ", shader_begin);

                const ShaderType shader_type = string_to_shader_type(params);
                const std::string_view shader_code = contents.substr(shader_begin, shader_end - shader_begin);

                ShaderData subshader;
                subshader.type = shader_type;
                subshader.contents = shader_code;
                results.shaders.push_back( std::move(subshader) );

                line_end = shader_end - 1;
            }

            line_begin = line_end + 1;
            line_end = contents.find('\n', line_begin);
        }

        return results;
    }

    ShaderID load_shader(const std::string& contents, ShaderType type)
    {
        const u32 shader_type = to_opengl_type(type);

        const ShaderID shader_id = glCreateShader(shader_type);
        const char* shader = contents.c_str();

        glShaderSource(shader_id, 1, &shader, nullptr);
        glCompileShader(shader_id);

        i32 results;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &results);
        if (results == GL_FALSE)
        {
            i32 length;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

            std::vector<GLchar> msg(length);
            glGetShaderInfoLog(shader_id, length, &length, msg.data());

            glDeleteProgram(shader_id);

            APORIA_LOG(Error, std::string{ msg.data() });

            return 0;
        }

        return shader_id;
    }

    void link_shaders(ShaderID program_id, const std::vector<ShaderID>& loaded_shaders)
    {
        for (const ShaderID shader_id : loaded_shaders)
        {
            glAttachShader(program_id, shader_id);
        }

        glLinkProgram(program_id);
        glValidateProgram(program_id);

        for (const ShaderID shader_id : loaded_shaders)
        {
            glDetachShader(program_id, shader_id);
            glDeleteShader(shader_id);
        }
    }

    void apply_shader_defaults(ShaderProperties& properties)
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

    void apply_shader_properties(ShaderID program_id)
    {
        APORIA_VALIDATE_OR_RETURN(programs.contains(program_id),
            "Shader program with ID = {} is not valid!", program_id);

        const ShaderInfo& program_info = programs.at(program_id);
        const ShaderProperties& properties = program_info.properties;

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

    i32 get_uniform_location(const std::string& name)
    {
        APORIA_VALIDATE_OR_RETURN_VALUE(programs.contains(active_program_id), -1,
            "No active shader program!");

        ShaderInfo& program_info = programs.at(active_program_id);
        std::unordered_map<std::string, i32>& locations_dict = program_info.locations;

        i32 location;
        if (!locations_dict.contains(name))
        {
            location = glGetUniformLocation(active_program_id, name.c_str());
            if (location == -1)
            {
                APORIA_LOG(Error, "'{}' does not correspond to an active uniform variable in program {}!", name, active_program_id);
            }
            else
            {
                locations_dict.emplace(name, location);
            }
        }
        else
        {
            location = locations_dict.at(name);
        }

        return location;
    }

    ShaderID create_shader(std::string_view filepath)
    {
        ShaderID program_id = glCreateProgram();

        std::vector<ShaderID> loaded_shaders;
        loaded_shaders.reserve(2);

        const std::string shader_contents = read_file(filepath);
        ShaderProgramData shader_data = parse_shader(shader_contents);
        for (const ShaderData& data : shader_data.shaders)
        {
            if (ShaderID shader_id = load_shader(data.contents, data.type))
            {
                loaded_shaders.push_back(shader_id);
            }
        }

        if (loaded_shaders.empty())
        {
            glDeleteProgram(program_id);
            return 0;
        }

        link_shaders(program_id, loaded_shaders);

        apply_shader_defaults(shader_data.properties);

        ShaderInfo program_info;
        program_info.source = filepath;
        program_info.properties = shader_data.properties;

        programs.emplace(program_id, std::move(program_info));

        return program_id;
    }

    void remove_shader(ShaderID program_id)
    {
        APORIA_VALIDATE_OR_RETURN(programs.contains(program_id),
            "Shader program with ID = {} is not valid!", program_id);

        glDeleteProgram(program_id);
        programs.erase(program_id);
    }

    void remove_all_shaders()
    {
        for (const ShaderID program_id : std::views::keys(programs))
        {
            glDeleteProgram(program_id);
        }

        programs.clear();
    }

    void set_default_shader_properties(ShaderProperties shader_properties)
    {
        default_shader_properties = shader_properties;
    }

    void reload_shader(ShaderID program_id)
    {
        APORIA_VALIDATE_OR_RETURN(programs.contains(program_id),
            "Shader program with ID = {} is not valid!", program_id);

        const std::string program_source = programs.at(program_id).source;

        glDeleteProgram(program_id);
        programs.erase(program_id);

        create_shader(program_source);
    }

    void bind_shader(ShaderID program_id)
    {
        assert(program_id > 0);
        apply_shader_properties(program_id);

        glUseProgram(program_id);
        active_program_id = program_id;
    }

    void unbind_shader()
    {
        glUseProgram(0);
        active_program_id = 0;
    }

    void shader_set_float(const std::string& name, f32 value)
    {
        glUniform1f(get_uniform_location(name), value);
    }

    void shader_set_float2(const std::string& name, v2 value)
    {
        glUniform2f(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_float3(const std::string& name, v3 value)
    {
        glUniform3f(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_float4(const std::string& name, v4 value)
    {
        glUniform4f(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_float_array(const std::string& name, f32* value, i32 count)
    {
        glUniform1fv(get_uniform_location(name), count, value);
    }

#if !defined(APORIA_EMSCRIPTEN)
    void shader_set_double(const std::string& name, f64 value)
    {
        glUniform1d(get_uniform_location(name), value);
    }

    void shader_set_double2(const std::string& name, v2_f64 value)
    {
        glUniform2d(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_double3(const std::string& name, v3_f64 value)
    {
        glUniform3d(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_double4(const std::string& name, v4_f64 value)
    {
        glUniform4d(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_double_array(const std::string& name, f64* value, i32 count)
    {
        glUniform1dv(get_uniform_location(name), count, value);
    }
#endif

    void shader_set_int(const std::string& name, i32 value)
    {
        glUniform1i(get_uniform_location(name), value);
    }

    void shader_set_int2(const std::string& name, v2_i32 value)
    {
        glUniform2i(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_int3(const std::string& name, v3_i32 value)
    {
        glUniform3i(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_int4(const std::string& name, v4_i32 value)
    {
        glUniform4i(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_int_array(const std::string& name, i32* value, i32 count)
    {
        glUniform1iv(get_uniform_location(name), count, value);
    }

    void shader_set_uint(const std::string& name, u32 value)
    {
        glUniform1ui(get_uniform_location(name), value);
    }

    void shader_set_uint2(const std::string& name, v2_u32 value)
    {
        glUniform2ui(get_uniform_location(name), value.x, value.y);
    }

    void shader_set_uint3(const std::string& name, v3_u32 value)
    {
        glUniform3ui(get_uniform_location(name), value.x, value.y, value.z);
    }

    void shader_set_uint4(const std::string& name, v4_u32 value)
    {
        glUniform4ui(get_uniform_location(name), value.x, value.y, value.z, value.w);
    }

    void shader_set_uint_array(const std::string& name, u32* value, i32 count)
    {
        glUniform1uiv(get_uniform_location(name), count, value);
    }

    void shader_set_mat2(const std::string& name, m2 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix2fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void shader_set_mat3(const std::string& name, m3 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix3fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }

    void shader_set_mat4(const std::string& name, m4 value, bool transpose /* = false */, i32 count /* = 1 */)
    {
        glUniformMatrix4fv(get_uniform_location(name), count, transpose ? GL_TRUE : GL_FALSE, &value[0][0]);
    }
}
