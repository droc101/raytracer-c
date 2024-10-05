#version 460

in vec2 VERTEX; // Model space vertex position

out vec2 UV; // Output UV coordinates for fragment shader

uniform mat4 WORLD_VIEW_MATRIX; // World to screen matrix

void main() {
    UV = vec2(VERTEX.x, VERTEX.y);
    gl_Position = WORLD_VIEW_MATRIX * vec4(VERTEX.x, -0.5, VERTEX.y, 1.0);
}