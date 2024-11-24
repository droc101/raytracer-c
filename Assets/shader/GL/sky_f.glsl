#version 460

in vec2 UV; // UV coordinates of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;
uniform vec4 col;

void main() {
    COLOR = texture(alb, UV).rgba * col;
}