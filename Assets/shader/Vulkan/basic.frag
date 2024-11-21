#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler2D textureSampler[];
layout(binding = 2) uniform DataBufferObject {
    uint textureIndex;
} data;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler[data.textureIndex], inUV);
}
