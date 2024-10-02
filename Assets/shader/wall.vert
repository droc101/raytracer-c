#version 460

in vec3 VERTEX;
in vec2 VERTEX_UV;

out vec2 UV;
uniform mat4 MODELVIEW_MATRIX;

void main() {
    UV = VERTEX_UV;
    gl_Position = MODELVIEW_MATRIX * vec4(VERTEX, 1.0);
}