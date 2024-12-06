#version 460

in vec2 UV; // UV coordinates of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;

void main() {
    COLOR.a = 1.0;
    COLOR.rgb = texture(alb, UV).rgb;
}