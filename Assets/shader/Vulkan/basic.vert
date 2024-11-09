#version 460

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextureCoordinate;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTextureCoordinate;
void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.x, 0.0, inPosition.y, 1.0);
    fragColor = inColor;
    fragTextureCoordinate = inTextureCoordinate;
}