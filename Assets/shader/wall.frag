#version 460

in vec2 UV;
flat in float shade;
out vec4 COLOR;

uniform sampler2D alb;
uniform vec3 fog_color;
uniform float fog_start;
uniform float fog_end;

void main() {
    COLOR = texture(alb, UV).rgba;
    COLOR.rgb *= vec3(shade);
    COLOR.a = 1.0f; // debug

    float fog_factor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog_start) / (fog_end - fog_start), 0.0, 1.0);
    COLOR.rgb = mix(COLOR.rgb, fog_color, fog_factor);
}