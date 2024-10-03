#version 460

in vec3 VERTEX;
in vec2 VERTEX_UV;

out vec2 UV;
uniform mat4 MODEL_WORLD_MATRIX;
uniform mat4 WORLD_VIEW_MATRIX;

void main() {
    UV = VERTEX_UV;
    gl_Position = WORLD_VIEW_MATRIX * MODEL_WORLD_MATRIX * vec4(VERTEX, 1.0);
}