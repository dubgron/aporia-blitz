// Override blending
#blend one zero
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

uniform sampler2D u_masking;
uniform mat4 u_vp_matrix;
uniform vec2 u_window_size;
uniform float u_camera_zoom;

out vec4 out_color;

#define MAX_STEPS (100000 / MAX_LIGHTS)
#define EPS 1e-4

float march_shadow(vec2 origin, float range, vec2 dir)
{
    vec2 aspect_ratio = u_window_size.xy / u_window_size.y;

    vec2 range_scaled = range * u_camera_zoom / aspect_ratio;
    float step = 1.0 / float(MAX_STEPS);
    float total_step = 0.0;
    for(int i = 0; i < MAX_STEPS; i++)
    {
        vec2 ray = dir * total_step * range_scaled;
        vec2 dest = origin + ray;

        float collision = texture(u_masking, dest).a;
        if (collision > EPS)
        {
            return length(total_step);
        }

        total_step += step;
    }

    return 1.0;
}

void main()
{
    int x = int(vs_tex_coord.x * u_window_size.x);
    int y = int(vs_tex_coord.y * u_window_size.y);

    if (y < int(u_num_lights))
    {
        Light light = lights[y];

        vec2 origin = vec2(u_vp_matrix * vec4(light.origin, 0.0, 1.0)) * 0.5 + vec2(0.5);

        float angle = vs_tex_coord.x * TAU;
        vec2 dir = -vec2(cos(angle), sin(angle));

        float dist = march_shadow(origin, light.range, dir);

        out_color = vec4(dist, 0.0, 0.0, 1.0);
    }
}
