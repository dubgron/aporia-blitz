#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 1) in vec4 l_color;
layout (location = 3) in vec2 l_tex_coord;
layout (location = 4) in float l_thickness;

uniform mat4 u_vp_matrix;

out vec4 out_color;

void main()
{
    vec3 position = l_position + vec3(l_tex_coord * l_thickness / 2.0, 0.0);
    gl_Position = u_vp_matrix * vec4(position, 1.0);

    out_color = l_color;
}



#type fragment
#version 450 core

out vec4 l_color;

in vec4 out_color;

void main()
{
    l_color = out_color;
}
