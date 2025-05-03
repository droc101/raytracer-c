#version 330 core

in vec2 UV; // UV coordinates of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;

void main() {
	if (texture(alb, UV).a < 0.5) {
		discard; // Discard the fragment if the alpha is less than 0.5
	}
    COLOR.a = 1.0;
    COLOR.rgb = texture(alb, UV).rgb;
}