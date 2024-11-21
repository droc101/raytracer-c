#version 460

layout(quads) in;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec3 inColor[];
layout (location = 1) in vec2 inUV[];

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * mix(
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


