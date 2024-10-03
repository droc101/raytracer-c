#version 460

in vec2 UV; // UV coordinates of the fragment
flat in float SHADE; // Shading value of this wall

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb; // Albedo texture of the wall
uniform vec3 fog_color; // Color of the fog
uniform float fog_start; // Start distance of the fog
uniform float fog_end; // End distance of the fog

void main() {
    COLOR = texture(alb, UV).rgba;
    COLOR.rgb *= vec3(SHADE);
    COLOR.a = 1.0f; // debug

    float fog_factor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog_start) / (fog_end - fog_start), 0.0, 1.0);
    COLOR.rgb = mix(COLOR.rgb, fog_color, fog_factor);
}