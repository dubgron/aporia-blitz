#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

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
layout (location = 3) out vec2 out_uv;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_position = in_position;
    out_color = in_color;
    out_tex_id = in_tex_id;
    out_uv = in_tex_coord;
}



#type fragment
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in flat int in_tex_id;
layout (location = 3) in vec2 in_uv;

uniform sampler2D u_atlas[32];
uniform vec2 u_light_position;
uniform vec3 u_light_color;

layout (location = 0) out vec4 out_color;

void main()
{
    vec4 object_color = in_color;

    if (in_tex_id > 0)
    {
        object_color *= texture(u_atlas[in_tex_id], in_uv);
    }

    out_color = object_color;
}
