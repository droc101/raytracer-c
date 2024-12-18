#version 460

layout(vertices=4) out;

layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in uint inTextureIndex[];

layout(location = 0) out vec4 outColor[4];
layout(location = 1) out vec2 outUV[4];
layout(location = 2) out uint outTextureIndex[4];

void main() {
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = 1.0;
        gl_TessLevelOuter[3] = 1.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    outColor[gl_InvocationID] = inColor[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];
    outTextureIndex[gl_InvocationID] = inTextureIndex[gl_InvocationID];
}