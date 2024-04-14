#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 3) in vec2 in_tex_coord;

layout (location = 0) out vec2 out_tex_coord;

void main()
{
    gl_Position = vec4(in_position, 1.0);

    out_tex_coord = in_tex_coord;
}



#type fragment
#version 450 core

layout (location = 0) in vec2 in_tex_coord;

uniform sampler2D u_framebuffer;

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = texture(u_framebuffer, in_tex_coord);
}
