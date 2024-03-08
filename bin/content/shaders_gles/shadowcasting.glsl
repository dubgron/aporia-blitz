// Multiplicative blending
#blend dst_color src_color
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

uniform sampler2D u_raymarching;
uniform mat4 u_vp_matrix;
uniform vec2 u_window_size;
uniform float u_camera_zoom;

out vec4 out_color;

float sample_shadow(int id, vec2 uv)
{
    float a = atan(uv.y, uv.x) / TAU + 0.5;
    float idn = float(id) / u_window_size.y;

    vec2 half_pixel_offset = 0.5 / u_window_size;

    float threshold = 0.02;

    float r = length(uv) / lights[id].range;
    if (r < 1.0 - threshold)
    {
        float s = texture(u_raymarching, vec2(a, idn) + half_pixel_offset).x;
        return 1.0 - smoothstep(s, s + threshold, r);
    }
    return 0.0;
}

vec3 mix_lights(vec2 uv)
{
    vec3 ambient = vec3(0.1);

    for (int i = 0; i < int(u_num_lights); i++)
    {
        Light light = lights[i];

        vec2 origin = vec2(u_vp_matrix * vec4(light.origin, 0.0, 1.0)) / 2.0;
        vec3 color = light.color * light.intensity;

        vec2 aspect_ratio = u_window_size.xy / u_window_size.y;

        vec2 from_light = (uv - origin) * aspect_ratio / u_camera_zoom;
        float dist = length(from_light);

        float attenuation = clamp(pow(1.0 - dist / light.range, light.falloff), 0.0, 1.0);

        ambient += color * sample_shadow(i, from_light) * attenuation;
    }

    return ambient;
}

void main()
{
    vec2 uv = (vs_tex_coord - vec2(0.5));
    out_color = vec4(mix_lights(uv), 1.0);
}
