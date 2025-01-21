#version 460

layout(binding = 0) uniform Mat4 {
    vec4 i;
    vec4 j;
    vec4 k;
    vec4 l;
} transform;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTransform0;
layout(location = 4) in vec4 inTransform1;
layout(location = 5) in vec4 inTransform2;
layout(location = 6) in vec4 inTransform3;
layout(location = 7) in uint inTextureIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;

void main() {
    gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * mat4(inTransform0, inTransform1, inTransform2, inTransform3) * vec4(inVertex, 1.0);
    outUV = inUV;
    outTextureIndex = inTextureIndex;
}