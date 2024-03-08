#type vertex
#version 300 es
precision highp float;

in vec3 in_position;
in vec4 in_color;
in float in_tex_unit;
in vec2 in_tex_coord;
in float in_screen_px_range;

uniform mat4 u_vp_matrix;
uniform float u_camera_zoom;

out vec3 vs_position;
out vec4 vs_color;
out float vs_tex_unit;
out vec2 vs_tex_coord;
out float vs_screen_px_range;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    vs_position = in_position;
    vs_color = in_color;
    vs_tex_unit = in_tex_unit;
    vs_tex_coord = in_tex_coord;
    vs_screen_px_range = in_screen_px_range * u_camera_zoom;
}



#type fragment
#version 300 es
precision highp float;

in vec3 vs_position;
in vec4 vs_color;
in float vs_tex_unit;
in vec2 vs_tex_coord;
in float vs_screen_px_range;

uniform sampler2D u_atlas[16];

out vec4 out_color;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

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
    vec3 msd = sample_texture().rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screen_px_distance = vs_screen_px_range * (sd - 0.5);
    float opacity = clamp(screen_px_distance + 0.5, 0.0, 1.0);
    out_color = vs_color * opacity;
}
