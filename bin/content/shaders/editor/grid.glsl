// Additive blending
#blend one one
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 450 core

layout (location = 0) in vec3 in_position;

uniform mat4 u_vp_matrix;

layout (location = 0) out vec2 out_position;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    vec4 unprojected = inverse(u_vp_matrix) * gl_Position;

    out_position = unprojected.xy;
}



#type fragment
#version 450 core

layout (location = 0) in vec2 in_position;

uniform float u_grid_size           = 100.0;
uniform float u_thick_size          = 10.0;

uniform float u_grid_thickness      = 1.0;
uniform float u_thick_thickness     = 2.0;
uniform float u_axis_thickness      = 2.0;

uniform float u_grid_brightness     = 0.25;
uniform float u_thick_brightness    = 0.5;
uniform float u_axis_brightness     = 1.0;

layout (location = 0) out vec4 out_color;
layout (location = 1) out int out_editor_index;

vec4 draw_grid(vec2 uv)
{
    vec3 color = vec3(0.0);

    // Normal grid
    {
        vec2 coord = uv / u_grid_size;
        vec2 derivative = fwidth(coord);

        // Draw grid
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
        float line = min(grid.x, grid.y) / u_grid_thickness;

        color += vec3(1.0 - min(line, 1.0)) * u_grid_brightness;
    }

    // XY axes and thicker grid
    {
        vec2 coord = uv / (u_grid_size * u_thick_size);
        vec2 derivative = fwidth(coord);

        // Draw XY axes
        vec2 axes = abs(coord) / derivative / u_axis_thickness;
        vec2 axis_line = 1.0 - min(axes.yx, 1.0);

        color += vec3(1.0, -1.0, -1.0) * axis_line.x * u_axis_brightness;
        color += vec3(-1.0, 1.0, -1.0) * axis_line.y * u_axis_brightness;

        // Draw thicker grid
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
        float line = min(grid.x, grid.y) / u_thick_thickness;

        color += vec3(1.0 - min(line, 1.0)) * u_thick_brightness;
    }

    return vec4(color, 1.0);
}

void main()
{
    out_color = draw_grid(in_position);
    out_editor_index = -1;
}
