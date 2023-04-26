#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 2) in int l_tex_id;
layout (location = 3) in vec2 l_tex_coord;

out DATA
{
    flat int tex_id;
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = vec4(l_position, 1.0);

    vs_out.tex_id = l_tex_id;
    vs_out.uv = l_tex_coord;
}


#type fragment
#version 450 core

out vec4 l_color;

in DATA
{
    flat int tex_id;
    vec2 uv;
} fs_in;

uniform sampler2D u_atlas[32];

const float offset_strengh = 1.0 / 400.0;

float max3(vec3 v)
{
    return max( max( v.x, v.y ), v.z );
}

void main()
{
    l_color = texture(u_atlas[fs_in.tex_id], fs_in.uv);
    // l_color = vec4(vec3(0.1/length(fs_in.uv-0.5)), 1.0);
    // const float max_color = max3(l_color.xyz);
    // const float threshold = step(max_color, 0.13);
    return;

    /* Regular */
    // float kernel[9] = float[](
    //     0.0, 0.0, 0.0,
    //     0.0, 1.0, 0.0,
    //     0.0, 0.0, 0.0
    // );

    /* Blur */
    // float kernel[9] = float[](
    //     1.0 / 16, 2.0 / 16, 1.0 / 16,
    //     2.0 / 16, 4.0 / 16, 2.0 / 16,
    //     1.0 / 16, 2.0 / 16, 1.0 / 16
    // );

    // /* Edge detection */
    // float kernel[9] = float[](
    //     1.0, 1.0, 1.0,
    //     1.0, -8.0, 1.0,
    //     1.0, 1.0, 1.0
    // );

    // vec3 sample_tex[9];
    // for(int i = 0; i < 9; i++)
    // {
    //     vec2 offset = vec2(i % 3, i / 3) * offset_strengh;
    //     sample_tex[i] = vec3(texture(u_atlas[fs_in.tex_id], fs_in.uv + offset));
    // }

    // vec3 color = vec3(0.0);
    // for(int i = 0; i < 9; i++)
    // {
    //     color += sample_tex[i] * kernel[i];
    // }

    // l_color = vec4(color, 1.0);
}
