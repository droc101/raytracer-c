#version 460

layout(quads) in;

layout(binding = 0) uniform Mat4 {
    vec4 i;
    vec4 j;
    vec4 k;
    vec4 l;
} transform;

layout (location = 0) in vec2 inUV[];

layout (location = 0) out vec2 outUV;

void main() {
    gl_Position = mat4(transform.i, transform.j, transform.k, transform.l) * mix(
                                                        mix(
                                                            gl_in[0].gl_Position,
                                                            gl_in[1].gl_Position,
                                                            gl_TessCoord.x
                                                        ),
                                                        mix(
                                                            gl_in[3].gl_Position,
                                                            gl_in[2].gl_Position,
                                                            gl_TessCoord.x
                                                        ),
                                                        gl_TessCoord.y
                                                    );

    outUV = mix(mix(inUV[0], inUV[1], gl_TessCoord.x), mix(inUV[3], inUV[2], gl_TessCoord.x), gl_TessCoord.y);
}
