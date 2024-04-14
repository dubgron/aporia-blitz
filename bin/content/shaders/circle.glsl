#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in float in_inner_radius;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out float out_inner_radius;

#if APORIA_EDITOR
layout (location = 5) in int in_editor_index;
layout (location = 3) out flat int out_editor_index;
#endif

void main()
{
    gl_Position = u_vp_matrix * vec4(in_position, 1.0);

    out_color = in_color;
    out_tex_coord = in_tex_coord;
    out_inner_radius = in_inner_radius;

#if APORIA_EDITOR
    out_editor_index = in_editor_index;
#endif
}



#type fragment
#version 450 core

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in float in_inner_radius;

layout (location = 0) out vec4 out_color;

#if APORIA_EDITOR
layout (location = 3) in flat int in_editor_index;
layout (location = 1) out int out_editor_index;
#endif

#define SMALL_NUMBER 0.01
#define PI 3.141592653589793

void main()
{
    float radius = length(in_tex_coord);

    float outer_radius = 1.0;
    float inner_radius = in_inner_radius;

    float outer_mask = smoothstep(outer_radius, outer_radius - SMALL_NUMBER, radius);
    float inner_mask = smoothstep(inner_radius - SMALL_NUMBER, inner_radius, radius);

    float circle_mask = outer_mask * inner_mask;

    out_color = in_color;
    out_color.a *= circle_mask;

#if APORIA_EDITOR
    out_editor_index = in_editor_index;
    if (circle_mask == 0.0)
        discard;
#endif
}
