// Multiplicative blending
#blend dst_color src_color
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in int in_tex_unit;
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

uniform sampler2D u_raycasting;
uniform mat4 u_vp_matrix;
uniform vec2 u_viewport_size;
uniform vec2 u_render_surface_size;
uniform float u_camera_zoom;

layout (location = 0) out vec4 out_color;

vec2 half_pixel_offset = 0.5 / u_viewport_size;

vec2 aspect_ratio = u_render_surface_size.xy / u_render_surface_size.y;
vec2 viewport_scale = u_viewport_size / u_render_surface_size;
vec2 range_scale = vec2(1.0) / u_viewport_size.y;
vec2 distance_normalization = u_camera_zoom * viewport_scale * range_scale / aspect_ratio;

float sample_shadow(int id, vec2 uv)
{
    float r = length(uv) / lights[id].range;

    float a = atan(uv.y, uv.x) / TAU + 0.5;
    float idn = float(id) / u_viewport_size.y;

    vec2 d = texture(u_raycasting, vec2(a, idn) + half_pixel_offset).xy;
    float s = d.r + (d.g / 255.0);

    return 1.0 - smoothstep(s, s + 0.01, r);
}
 
vec3 mix_lights(vec2 uv)
{
    vec3 ambient = vec3(0.1);

    for (int i = 0; i < u_num_lights; i++)
    {
        Light light = lights[i];

        vec2 origin = vec2(u_vp_matrix * vec4(light.origin, 0.0, 1.0)) / 2.0;
        vec3 color = light.color * light.intensity;

        vec2 from_light = (uv - origin) / distance_normalization;
        float dist = length(from_light);

        float attenuation = clamp(pow(1.0 - dist / light.range, light.falloff), 0.0, 1.0);

        ambient += color * sample_shadow(i, from_light) * attenuation;
    }

    return ambient;
}

void main()
{
    vec2 uv = (in_tex_coord - vec2(0.5));
    out_color = vec4(mix_lights(uv), 1.0);
}
