#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 1) in vec4 l_color;
layout (location = 3) in vec2 l_tex_coord;

uniform mat4 u_vp_matrix;

out DATA
{
    vec3 position;
    vec4 color;
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = u_vp_matrix * vec4(l_position, 1.0);

    vs_out.position = l_position;
    vs_out.color = l_color;
    vs_out.uv = l_tex_coord;
}



#type fragment
#version 450 core

out vec4 l_color;

in DATA
{
    vec3 position;
    vec4 color;
    vec2 uv;
} fs_in;

#define SMALL_NUMBER 0.01

void main()
{
    l_color = fs_in.color;
    l_color.a = 1.0 - smoothstep(1.0 - SMALL_NUMBER, 1.0, length(fs_in.uv));
}
