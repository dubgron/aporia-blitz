#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 3) in vec2 in_tex_coord;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_tex_coord;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_color = in_color;
    out_tex_coord = in_tex_coord;
}



#type fragment
#version 450 core

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 0) out vec4 out_color;

#define SMALL_NUMBER 0.01

void main()
{
    float circle_mask = 1.0 - smoothstep(1.0 - SMALL_NUMBER, 1.0, length(in_tex_coord));

    out_color = in_color;
    out_color.a *= circle_mask;
}
