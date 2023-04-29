#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in int in_tex_id;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in float in_screen_px_range;

uniform mat4 u_vp_matrix;
uniform float u_camera_zoom;

layout (location = 0) out vec3 out_position;
layout (location = 1) out vec4 out_color;
layout (location = 2) out flat int out_tex_id;
layout (location = 3) out vec2 out_uv;
layout (location = 4) out float out_screen_px_range;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_position = in_position;
    out_color = in_color;
    out_tex_id = in_tex_id;
    out_uv = in_tex_coord;
    out_screen_px_range = in_screen_px_range * u_camera_zoom;
}



#type fragment
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in flat int in_tex_id;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in float in_screen_px_range;

uniform sampler2D u_atlas[32];

layout (location = 0) out vec4 out_color;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msd = texture(u_atlas[in_tex_id], in_uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screen_px_distance = in_screen_px_range * (sd - 0.5);
    float opacity = clamp(screen_px_distance + 0.5, 0.0, 1.0);
    out_color = in_color * opacity;
}
