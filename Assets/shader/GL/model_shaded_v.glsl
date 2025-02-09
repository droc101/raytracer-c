#version 330 core

in vec3 VERTEX; // Model space vertex position
in vec2 VERTEX_UV; // Vertex UV coordinates
in vec3 VERTEX_NORMAL; // Vertex normal

out vec2 UV; // Output UV coordinates for fragment shader
out vec3 NORMAL; // Output normal for fragment shader

uniform mat4 MODEL_WORLD_MATRIX; // Model to world matrix

layout(std140) uniform SharedUniforms
{
    mat4 worldViewMatrix;
    vec3 fogColor;
    float fogStart;
    float fogEnd;
    float cameraYaw;
} uniforms;

void main() {
    UV = VERTEX_UV;
    NORMAL = VERTEX_NORMAL;
    gl_Position = uniforms.worldViewMatrix * MODEL_WORLD_MATRIX * vec4(VERTEX, 1.0);
}