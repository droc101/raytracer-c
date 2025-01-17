#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstants {
    layout(offset = 12) uint color;
} pushConstants;

layout(binding = 1) uniform sampler2D textureSampler[];

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    if (inTextureIndex == (pushConstants.color & 0xFF)) {
        outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV) * vec4(unpackUnorm4x8(pushConstants.color).bgr, 1);
    } else {
        outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
    }
    if (outColor.a < 0.5) discard;
    outColor.a = 1.0;
}
