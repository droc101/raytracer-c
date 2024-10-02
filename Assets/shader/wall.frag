#version 460

in vec2 UV;
out vec4 COLOR;

uniform sampler2D alb;
uniform vec3 fog_color;
uniform float fog_start;
uniform float fog_end;

void main() {
    COLOR = texture(alb, UV).rgba;
}