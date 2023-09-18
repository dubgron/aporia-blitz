#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;
in float in_tex_unit;
in vec2 in_tex_coord;

out vec2 vs_tex_coord;

void main()
{
    gl_Position = vec4(in_position, 1.0);

    vs_tex_coord = in_tex_coord;
}



#type fragment
#version 300 es
precision highp float;

in vec2 vs_tex_coord;

uniform sampler2D u_framebuffer;

out vec4 out_color;

void main()
{
    out_color = texture(u_framebuffer, vs_tex_coord);
}
