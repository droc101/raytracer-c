#version 460

in vec3 VERTEX;
in vec2 VERTEX_UV;

out vec2 UV;
flat out float shade;

uniform mat4 MODEL_WORLD_MATRIX;
uniform mat4 WORLD_VIEW_MATRIX;
uniform float camera_yaw;
uniform float wall_angle;

const float PI = 3.14159265359;

void main() {
    UV = VERTEX_UV;
    gl_Position = WORLD_VIEW_MATRIX * MODEL_WORLD_MATRIX * vec4(VERTEX, 1.0);

    shade = abs(cos((camera_yaw + (1.5 * PI)) - wall_angle));
    shade = max(0.6, min(1, shade));
}