#version 460

layout(quads) in;

layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in uint inTextureIndex[];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out uint outTextureIndex;

void main() {
    gl_Position = mix(
        mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
        mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
        gl_TessCoord.y
    );

    outColor = mix(mix(inColor[0], inColor[1], gl_TessCoord.x), mix(inColor[3], inColor[2], gl_TessCoord.x), gl_TessCoord.y);
    outUV = mix(mix(inUV[0], inUV[1], gl_TessCoord.x), mix(inUV[3], inUV[2], gl_TessCoord.x), gl_TessCoord.y);
    outTextureIndex = inTextureIndex[0]; // TODO guh
}
