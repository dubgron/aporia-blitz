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

uniform float u_time;

layout (location = 0) out vec4 out_color;

#define SMALL_NUMBER 0.4

void main()
{
    float radius = length(in_tex_coord);

    // out_color = in_color * sin(u_time * 10.0);
    float t = sin(radius * 25.0 - u_time * 12.0);
    out_color.rgb = vec3(0.980,0.036,0.234) * t;
    out_color.a = 1.0 - smoothstep(1.0 - SMALL_NUMBER, 1.0, radius);
    out_color.a *= t;
}
