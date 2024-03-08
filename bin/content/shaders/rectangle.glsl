#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec4 out_color;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_color = in_color;
}



#type fragment
#version 450 core

layout (location = 0) in vec4 in_color;
layout (location = 0) out vec4 out_color;

void main()
{
    out_color = in_color;
}
