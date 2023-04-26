#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 450 core

layout (location = 0) in vec3 l_position;
layout (location = 1) in vec4 l_color;
layout (location = 2) in int l_tex_id;
layout (location = 3) in vec2 l_tex_coord;

uniform mat4 u_vp_matrix;

out DATA
{
    vec3 position;
    vec4 color;
    flat int tex_id;
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = u_vp_matrix * vec4(l_position, 1.0);

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

uniform sampler2D u_atlas[32];
uniform vec2 u_light_position;
uniform vec3 u_light_color;

void main()
{
    vec4 object_color = fs_in.color;

    /* Handle quads with a texture */
    if (fs_in.tex_id > 0)
    {
        object_color *= texture(u_atlas[fs_in.tex_id], fs_in.uv);
    }
    /* Handle circles */
    else if (length(fs_in.uv) > 1.0)
    {
        // object_color.a = 0.0;
    }

    l_color = object_color;

    // const vec3 vec_to_light = vec3(u_light_position - fs_in.position.xy, 1.0);

    // /* Ambient Lighting */
    // const float ambient_strength = 0.2;

    // const vec3 ambient = ambient_strength * u_light_color;

    // /* Diffuse Lighting */
    // const vec2 normal_uv = fs_in.uv + vec2(0.5, 0.0);
    // const vec3 normal_tex = texture(u_atlas[fs_in.tex_id], normal_uv).rgb;
    // const vec3 normal = normalize(normal_tex * 2.0 - 1.0);
    // const vec3 dir_to_light = normalize(vec_to_light);
    // const float diff = max(dot(normal, dir_to_light), 0.0);

    // const vec3 diffuse = diff * u_light_color;

    // /* Attenuation */
    // const vec3 attenuation_strength = vec3(0.005);

    // const float dist = length(vec_to_light);
	// const float attenuation = 1.0 / dot(attenuation_strength, vec3(1, dist, dist * dist));

    // l_color = vec4(ambient + attenuation * diffuse, 1.0) * object_color;
}
