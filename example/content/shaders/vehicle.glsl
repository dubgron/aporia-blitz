#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in int in_tex_id;
layout (location = 3) in vec2 in_tex_coord;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec3 out_position;
layout (location = 1) out vec4 out_color;
layout (location = 2) out flat int out_tex_id;
layout (location = 3) out vec2 out_tex_coord;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_position = in_position;
    out_color = in_color;
    out_tex_id = in_tex_id;
    out_tex_coord = in_tex_coord;
}



#type fragment
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in flat int in_tex_id;
layout (location = 3) in vec2 in_tex_coord;

uniform sampler2D u_atlas[32];
uniform vec2 u_forward;

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

layout (location = 0) out vec4 out_color;

vec3 binormal(vec2 forward, vec3 v)
{
    float x = forward.y * v.x + forward.x * v.y;
    float y = -forward.x * v.x + forward.y * v.y;
    float z = v.z;
    return vec3(x, y, z);
}

void main()
{
    vec4 object_color = in_color;

    if (in_tex_id > 0)
    {
        object_color *= texture(u_atlas[in_tex_id], in_tex_coord);
    }

    /* Ambient Lighting */
    float ambient_strength = 0.3;

    vec3 lights_combined = vec3(ambient_strength);

    for (int i = 0; i < u_num_lights; ++i)
    {
        Light light = lights[i];

        vec3 vec_to_light = vec3(light.origin - in_position.xy, 1.0);
        vec3 dir_to_light = normalize(vec_to_light);
        float dist_to_light = length(vec_to_light);

        /* Diffuse Lighting */
        vec2 normal_uv = in_tex_coord + vec2(0.5, 0.0);
        vec3 normal_tex = texture(u_atlas[in_tex_id], normal_uv).rgb;
        vec3 normal = normalize(normal_tex * 2.0 - 1.0);
        normal = binormal(u_forward, normal);

        float diffuse = 0.5 * max(dot(normal, dir_to_light), 0.0);

        lights_combined += diffuse * light.color;
    } 


    out_color = vec4(lights_combined, 1.0) * object_color;
}
