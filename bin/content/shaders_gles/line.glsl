#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;
in float in_tex_unit;
in vec2 in_normal;
in float in_thickness;

uniform mat4 u_vp_matrix;

out vec4 vs_color;

void main()
{
    vec3 position = in_position + vec3(in_normal * in_thickness / 2.0, 0.0);
    gl_Position = u_vp_matrix * vec4(position, 1.0);

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
