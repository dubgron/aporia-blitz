#blend src_alpha one_minus_src_alpha
#blend_op add
#depth_test lequal
#depth_write on

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in int in_tex_unit;
layout (location = 3) in vec2 in_tex_coord;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec4 out_color;
layout (location = 1) out flat int out_tex_unit;
layout (location = 2) out vec2 out_tex_coord;

#if APORIA_EDITOR
layout (location = 5) in int in_editor_index;
layout (location = 3) out flat int out_editor_index;
#endif

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_color = in_color;
    out_tex_unit = in_tex_unit;
    out_tex_coord = in_tex_coord;

#if APORIA_EDITOR
    out_editor_index = in_editor_index;
#endif
}



#type fragment
#version 450 core

layout (location = 0) in vec4 in_color;
layout (location = 1) in flat int in_tex_unit;
layout (location = 2) in vec2 in_tex_coord;

uniform sampler2D u_atlas[32];

layout (location = 0) out vec4 out_color;

#if APORIA_EDITOR
layout (location = 3) in flat int in_editor_index;
layout (location = 1) out int out_editor_index;
#endif

void main()
{
    vec4 object_color = in_color;
    object_color *= texture(u_atlas[in_tex_unit], in_tex_coord);
    out_color = object_color;

#if APORIA_EDITOR
    out_editor_index = in_editor_index;
    if (out_color.a == 0.0)
        discard;
#endif
}
