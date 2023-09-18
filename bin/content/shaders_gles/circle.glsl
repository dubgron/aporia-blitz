#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;
in float in_tex_unit;
in vec2 in_tex_coord;

uniform mat4 u_vp_matrix;

out vec4 vs_color;
out vec2 vs_tex_coord;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    vs_color = in_color;
    vs_tex_coord = in_tex_coord;
}



#type fragment
#version 300 es
precision highp float;

in vec4 vs_color;
in vec2 vs_tex_coord;

out vec4 out_color;

#define SMALL_NUMBER 0.01

void main()
{
    out_color = vs_color;
    out_color.a = 1.0 - smoothstep(1.0 - SMALL_NUMBER, 1.0, length(vs_tex_coord));
}
