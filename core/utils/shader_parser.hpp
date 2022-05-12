#pragma once

#include <algorithm>
#include <string>

#include "graphics/shader_properties.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    inline ShaderData parse_shader(const std::string& filepath)
    {
        ShaderData results;

        /* Most common case - parsing vertex and fragment shaders */
        results.subshaders.reserve(2);

        const std::string contents = read_file(filepath);

        const std::string_view type_token = "#type ";
        const std::string_view version_token = "#version ";

        size_t line_begin = 0;
        size_t line_end = contents.find('\n', line_begin);

        while (line_end != std::string::npos)
        {
            const std::string line = contents.substr(line_begin, line_end - line_begin);

            const size_t params_begin = line.find(' ') + 1;
            const std::string params = line.substr(params_begin, line_end - params_begin);
            if (line.starts_with("#blend "))
            {
                const size_t delim = params.find(' ');
                if (delim != std::string::npos)
                {
                    results.properties.blend[0] = string_to<ShaderBlend>(params.substr(0, delim));
                    results.properties.blend[1] = string_to<ShaderBlend>(params.substr(delim + 1, line_end - delim - 1));
                }
                else
                {
                    results.properties.blend[0] = string_to<ShaderBlend>(params);
                }
            }
            else if (line.starts_with("#blend_op "))
            {
                results.properties.blend_op = string_to<ShaderBlendOp>(params);
            }
            else if (line.starts_with("#depth_test "))
            {
                results.properties.depth_test = string_to<ShaderDepthTest>(params);
            }
            else if (line.starts_with("#depth_write "))
            {
                results.properties.depth_write = string_to<ShaderDepthWrite>(params);
            }
            else if (line.starts_with("#type "))
            {
                const size_t shader_begin = line_end + 1;
                const size_t shader_end = contents.find("#type ", shader_begin);

                ShaderType type = string_to<ShaderType>(params);
                std::string shader = contents.substr(shader_begin, shader_end - shader_begin);

                results.subshaders.emplace_back( std::move(type), std::move(shader) );

                line_end = shader_end - 1;
            }

            line_begin = line_end + 1;
            line_end = contents.find('\n', line_begin);
        }

        return results;
    }
}
