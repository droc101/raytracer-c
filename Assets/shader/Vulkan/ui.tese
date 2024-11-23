#version 460

layout(quads) in;

layout (location = 0) in vec4 inColor[];

layout (location = 0) out vec4 outColor;

void main() {
    gl_Position = mix(
        mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
        mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
        gl_TessCoord.y
    );

    outColor = mix(mix(inColor[0], inColor[1], gl_TessCoord.x), mix(inColor[3], inColor[2], gl_TessCoord.x), gl_TessCoord.y);
}
