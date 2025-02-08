#version 330 core

in vec3 VERTEX; // Model space vertex position
in vec2 VERTEX_UV; // Vertex UV coordinates

out vec2 UV; // Output UV coordinates for fragment shader
flat out float SHADE; // Output shade value for fragment shader

uniform mat4 MODEL_WORLD_MATRIX; // Model to world matrix

layout(std140) uniform SharedUniforms
{
    mat4 worldViewMatrix;
    vec3 fogColor;
    float fogStart;
    float fogEnd;
    float cameraYaw;
} uniforms;

uniform float wall_angle; // Wall angle

const float PI = 3.14159265359; // yummy

void main() {
    UV = VERTEX_UV;
    gl_Position = uniforms.worldViewMatrix * MODEL_WORLD_MATRIX * vec4(VERTEX, 1.0);

    SHADE = abs(cos((uniforms.cameraYaw + (1.5 * PI)) - wall_angle));
    SHADE = max(0.6, min(1, SHADE));
}