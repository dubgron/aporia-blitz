#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 1) in vec4 l_color;
layout (location = 2) in int l_tex_id;
layout (location = 3) in vec2 l_tex_coord;
layout (location = 4) in float l_screen_px_range;

uniform mat4 u_vp_matrix;
uniform float u_camera_zoom;

out DATA
{
    vec3 position;
    vec4 color;
    flat int tid;
    vec2 uv;
    float screen_px_range;
} vs_out;

void main()
{
    gl_Position = u_vp_matrix * vec4(l_position, 1.0);

    vs_out.position = l_position;
    vs_out.color = l_color;
    vs_out.tid = l_tex_id;
    vs_out.uv = l_tex_coord;
    vs_out.screen_px_range = l_screen_px_range * u_camera_zoom;
}



#type fragment
#version 450 core

out vec4 l_color;

in DATA
{
    vec3 position;
    vec4 color;
    flat int tid;
    vec2 uv;
    float screen_px_range;
} fs_in;

uniform sampler2D u_atlas[32];

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msd = texture(u_atlas[fs_in.tid], fs_in.uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screen_px_distance = fs_in.screen_px_range * (sd - 0.5);
    float opacity = clamp(screen_px_distance + 0.5, 0.0, 1.0);
    l_color = fs_in.color * opacity;
}
