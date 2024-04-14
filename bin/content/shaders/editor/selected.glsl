// Additive blending
#blend one one
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in int in_tex_unit;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 5) in int in_editor_index;

uniform mat4 u_vp_matrix;
uniform float u_time_since_selected;

layout (location = 0) out vec4 out_color;
layout (location = 1) out flat int out_tex_unit;
layout (location = 2) out vec2 out_uv;
layout (location = 3) out flat int out_editor_index;

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    vec4 selection_color = vec4(0.7, 0.7, 0.0, 0.2);
    float blinking = sin(u_time_since_selected * 5.0);

    out_color = in_color * selection_color * blinking;
    out_tex_unit = in_tex_unit;
    out_uv = in_tex_coord;
    out_editor_index = in_editor_index;
}



#type fragment
#version 450 core

layout (location = 0) in vec4 in_color;
layout (location = 1) in flat int in_tex_unit;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in flat int in_editor_index;

uniform sampler2D u_atlas[32];

layout (location = 0) out vec4 out_color;
layout (location = 1) out int out_editor_index;

void main()
{
    float texture_alpha = texture(u_atlas[in_tex_unit], in_uv).a;
    out_color = in_color * texture_alpha;

    out_editor_index = in_editor_index;
    if (texture_alpha == 0.0)
        discard;
}
