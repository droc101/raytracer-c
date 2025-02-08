#version 330 core

in vec3 VERTEX; // Model space vertex position
in vec2 VERTEX_UV; // Vertex UV coordinates
in vec3 VERTEX_NORMAL; // Vertex normal

out vec2 UV; // Output UV coordinates for fragment shader
out vec3 NORMAL; // Output normal for fragment shader

uniform mat4 MODEL_WORLD_MATRIX; // Model to world matrix
uniform mat4 WORLD_VIEW_MATRIX; // World to screen matrix

void main() {
    UV = VERTEX_UV;
    NORMAL = VERTEX_NORMAL;
    gl_Position = WORLD_VIEW_MATRIX * MODEL_WORLD_MATRIX * vec4(VERTEX, 1.0);
}