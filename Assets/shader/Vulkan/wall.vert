#version 460

layout(location = 0) in vec4 inVertex;
layout(location = 1) in float inHalfHeight;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec4 outVertex;
layout(location = 1) out float outHalfHeight;
layout(location = 2) out uint outTextureIndex;

void main() {
    gl_Position = vec4(inVertex.x, 0.0, inVertex.y, 1.0);
    outVertex = inVertex;
    outHalfHeight = inHalfHeight;
    outTextureIndex = inTextureIndex;
}