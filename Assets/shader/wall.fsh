#version 460

// Input from vertex shader
in vec2 UV;

// Output to fragment shader
out vec4 COLOR;

// Uniforms
uniform sampler2D alb;

uniform vec3 fog_color;
uniform float fog_start;
uniform float fog_end;

void main() {
    COLOR = texture(alb, UV).rgba;
}