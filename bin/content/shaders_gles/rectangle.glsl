#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;

uniform mat4 u_vp_matrix;

out vec4 vs_color;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    vs_color = in_color;
}



#type fragment
#version 300 es
precision highp float;

in vec4 vs_color;

out vec4 out_color;

void main()
{
    out_color = vs_color;
}
