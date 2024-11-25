#version 460

layout(location = 0) in vec4 in_posXY_uvZW;
layout(location = 1) in vec4 inColor;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out uint outTextureIndex;

void main() {
    gl_Position = vec4(in_posXY_uvZW.xy, 0.0, 1.0);
    outColor = inColor;
    outUV = in_posXY_uvZW.zw;
    outTextureIndex = inTextureIndex;
}