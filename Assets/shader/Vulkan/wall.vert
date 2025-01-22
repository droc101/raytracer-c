#version 460

layout(push_constant) uniform PushConstants {
    layout(offset = 0) vec2 playerPosition;
    layout(offset = 8) uint skyVertexCount;
    layout(offset = 12) uint skyTextureIndex;
} pushConstants;

layout(binding = 0) uniform Mat4 {
    vec4 i;
    vec4 j;
    vec4 k;
    vec4 l;
} transform;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;

void main() {
    if (pushConstants.skyVertexCount == 0) {
        if (gl_VertexIndex < 8) {
            gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * (vec4(inVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
            outUV = inUV + pushConstants.playerPosition;
        } else {
            gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * vec4(inVertex, 1.0);
            outUV = inUV;
        }
        outTextureIndex = inTextureIndex;
    } else {
        if (gl_VertexIndex < pushConstants.skyVertexCount + 4) {
            gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * (vec4(inVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
            if (pushConstants.skyVertexCount <= gl_VertexIndex) {
                outUV = inUV + pushConstants.playerPosition;
                outTextureIndex = inTextureIndex;
            } else {
                outUV = inUV;
                outTextureIndex = pushConstants.skyTextureIndex;
            }
        } else {
            gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * vec4(inVertex, 1.0);
            outUV = inUV;
            outTextureIndex = inTextureIndex;
        }
    }
}