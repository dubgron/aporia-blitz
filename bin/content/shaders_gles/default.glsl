#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;
in float in_tex_unit;
in vec2 in_tex_coord;

uniform mat4 u_vp_matrix;

out vec4 vs_color;
out float vs_tex_unit;
out vec2 vs_tex_coord;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    vs_color = in_color;
    vs_tex_unit = in_tex_unit;
    vs_tex_coord = in_tex_coord;
}



#type fragment
#version 300 es
precision highp float;

in vec4 vs_color;
in float vs_tex_unit;
in vec2 vs_tex_coord;

uniform sampler2D u_atlas[16];

out vec4 out_color;

vec4 sample_texture()
{
    int tex_unit = int(vs_tex_unit);
    if (tex_unit == 0)
        return texture(u_atlas[0], vs_tex_coord);
    else if (tex_unit == 1)
        return texture(u_atlas[1], vs_tex_coord);
    else if (tex_unit == 2)
        return texture(u_atlas[2], vs_tex_coord);
    else if (tex_unit == 3)
        return texture(u_atlas[3], vs_tex_coord);
    else if (tex_unit == 4)
        return texture(u_atlas[4], vs_tex_coord);
    else if (tex_unit == 5)
        return texture(u_atlas[5], vs_tex_coord);
    else if (tex_unit == 6)
        return texture(u_atlas[6], vs_tex_coord);
    else if (tex_unit == 7)
        return texture(u_atlas[7], vs_tex_coord);
    else if (tex_unit == 8)
        return texture(u_atlas[8], vs_tex_coord);
    else if (tex_unit == 9)
        return texture(u_atlas[9], vs_tex_coord);
    else if (tex_unit == 10)
        return texture(u_atlas[10], vs_tex_coord);
    else if (tex_unit == 11)
        return texture(u_atlas[11], vs_tex_coord);
    else if (tex_unit == 12)
        return texture(u_atlas[12], vs_tex_coord);
    else if (tex_unit == 13)
        return texture(u_atlas[13], vs_tex_coord);
    else if (tex_unit == 14)
        return texture(u_atlas[14], vs_tex_coord);
    else if (tex_unit == 15)
        return texture(u_atlas[15], vs_tex_coord);
    else
        return vec4(1.0, 0.0, 1.0, 1.0);
}

void main()
{
    vec4 texture = sample_texture();
    vec4 object_color = vs_color;

    out_color = vs_color * texture;
}
