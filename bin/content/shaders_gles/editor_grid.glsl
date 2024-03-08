#blend one one
#blend_op add
#depth_test off
#depth_write off

#type vertex
#version 300 es
precision highp float;

in vec3 in_position;

uniform mat4 u_vp_matrix;

out vec2 vs_position;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    vec4 unprojected = inverse(u_vp_matrix) * gl_Position;

    vs_position = unprojected.xy;
}



#type fragment
#version 300 es
precision highp float;

in vec2 vs_position;

float grid_size         = 100.0;
float thick_size        = 10.0;

float grid_thickness    = 1.0;
float thick_thickness   = 2.0;
float axis_thickness    = 2.0;

float grid_brightness   = 0.25;
float thick_brightness  = 0.5;
float axis_brightness   = 1.0;

out vec4 out_color;

vec4 draw_grid(vec2 uv)
{
    vec3 color = vec3(0.0);

    // Normal grid
    {
        vec2 coord = uv / grid_size;
        vec2 derivative = fwidth(coord);

        // Draw grid
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
        float line = min(grid.x, grid.y) / grid_thickness;

        color += vec3(1.0 - min(line, 1.0)) * grid_brightness;
    }

    // XY axes and thicker grid
    {
        vec2 coord = uv / (grid_size * thick_size);
        vec2 derivative = fwidth(coord);

        // Draw XY axes
        vec2 axes = abs(coord) / derivative / axis_thickness;
        vec2 axis_line = 1.0 - min(axes.yx, 1.0);

        color += vec3(1.0, -1.0, -1.0) * axis_line.x * axis_brightness;
        color += vec3(-1.0, 1.0, -1.0) * axis_line.y * axis_brightness;

        // Draw thicker grid
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
        float line = min(grid.x, grid.y) / thick_thickness;

        color += vec3(1.0 - min(line, 1.0)) * thick_brightness;
    }

    return vec4(color, 1.0);
}

void main()
{
    out_color = draw_grid(vs_position);
}
