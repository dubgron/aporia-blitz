// Override blending
#blend one zero
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 1) in vec4 l_color;
layout (location = 2) in int l_tex_id;
layout (location = 3) in vec2 l_tex_coord;

out DATA
{
    vec3 position;
    vec4 color;
    flat int tex_id;
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = vec4(l_position, 1.0);

    vs_out.position = l_position;
    vs_out.color = l_color;
    vs_out.tex_id = l_tex_id;
    vs_out.uv = l_tex_coord;
}



#type fragment
#version 450 core

out vec4 l_color;

in DATA
{
    vec3 position;
    vec4 color;
    flat int tex_id;
    vec2 uv;
} fs_in;

struct Light
{
    vec2 origin;
    float range;
    float falloff;

    vec3 color;
    float intensity;
};

#define TAU 6.2831853076

#define MAX_LIGHTS 100
layout (std140) uniform Lights
{
    Light lights[MAX_LIGHTS];
};

uniform uint u_num_lights;

uniform sampler2D u_masking;
uniform mat4 u_vp_matrix;
uniform vec2 u_window_size;
uniform float u_camera_zoom;

#define MAX_STEPS (100000 / MAX_LIGHTS)
#define EPS 1e-4

vec2 half_pixel_offset = 0.5 / u_window_size;
vec2 aspect_ratio = u_window_size.xy / u_window_size.y;

float march_shadow(vec2 origin, float range, vec2 dir)
{
    vec2 range_scaled = range * u_camera_zoom / aspect_ratio;
    float step = 1.0 / MAX_STEPS;
    float total_step = 0.0;
    for(int i = 0; i < MAX_STEPS; i++)
    {
        vec2 ray = dir * total_step * range_scaled;
        vec2 dest = origin + ray;

        // if (dest.x > 0.0 && dest.x < 1.0 && dest.y > 0.0 && dest.y < 1.0)
        {
            float collision = texture(u_masking, dest).a;
            if (collision > EPS)
            {
                return length(total_step);
            }
        }

        total_step += step;
    }

    return 1.0;
}

void main()
{
    int x = int(fs_in.uv.x * u_window_size.x);
    int y = int(fs_in.uv.y * u_window_size.y);

    if (y < u_num_lights)
    {
        Light light = lights[y];

        vec2 origin = vec2(u_vp_matrix * vec4(light.origin, 0.0, 1.0)) * 0.5 + vec2(0.5);

        float angle = fs_in.uv.x * TAU;
        vec2 dir = -vec2(cos(angle), sin(angle));

        float dist = march_shadow(origin, light.range, dir);

        l_color = vec4(dist, 0.0, 0.0, 1.0);
    }
}
