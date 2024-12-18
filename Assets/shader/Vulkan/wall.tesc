#version 460

layout(vertices=4) out;

layout(location = 0) in vec4 inVertex[];
layout(location = 1) in float inHalfHeight[];
layout(location = 2) in uint inTextureIndex[];

layout(location = 0) out vec2 outUV[4];
layout(location = 1) out uint outTextureIndex[4];

vec4 startVertex = inVertex[0];
vec4 endVertex = inVertex[1];
float halfHeight = inHalfHeight[0];

void main() {
    gl_TessLevelOuter[gl_InvocationID] = 1.0;
    if (gl_InvocationID == 0) {
        gl_out[gl_InvocationID].gl_Position = vec4(startVertex.x, halfHeight, startVertex.y, 1);
        outUV[gl_InvocationID] = startVertex.zw;
    } else if (gl_InvocationID == 1) {
        gl_out[gl_InvocationID].gl_Position = vec4(endVertex.x, halfHeight, endVertex.y, 1);
        outUV[gl_InvocationID] = vec2(endVertex.z, startVertex.w);
    } else if (gl_InvocationID == 2) {
        gl_out[gl_InvocationID].gl_Position = vec4(endVertex.x, -halfHeight, endVertex.y, 1);
        outUV[gl_InvocationID] = endVertex.zw;
    } else if (gl_InvocationID == 3) {
        gl_out[gl_InvocationID].gl_Position = vec4(startVertex.x, -halfHeight, startVertex.y, 1);
        outUV[gl_InvocationID] = vec2(startVertex.z, endVertex.w);
    }

    outTextureIndex[gl_InvocationID] = inTextureIndex[0];
}