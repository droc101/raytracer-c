#version 460 core

/**
 * This shader is used to render a a blurry version of the framebuffer.
 */


// Output to fragment shader
out vec4 COLOR;

// Uniforms
uniform sampler2D alb;
uniform int blurRadius; // Radius of the blur

void main() {
    vec2 uv = gl_FragCoord.xy / textureSize(alb, 0);

    // Box blur
    vec3 color = vec3(0.0);
    ivec2 tex_size = textureSize(alb, 0);
    for (int x = -blurRadius; x <= blurRadius; x++) {
        for (int y = -blurRadius; y <= blurRadius; y++) {
            color += texture(alb, uv + vec2(x, y) / tex_size).rgb;
        }
    }
    color /= (2.0 * blurRadius + 1.0) * (2.0 * blurRadius + 1.0);
    COLOR = vec4(color, 1.0);
}
