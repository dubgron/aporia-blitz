#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 3) in vec2 in_normal;
layout (location = 4) in float in_thickness;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec4 out_color;

void main()
{
    vec3 position = in_position + vec3(in_normal * in_thickness / 2.0, 0.0);
    gl_Position = u_vp_matrix * vec4(position, 1.0);

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
