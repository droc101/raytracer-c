#version 460 core

/**
 * This shader is used to render a a blurry version of the framebuffer.
 */

// Input from OpenGL
in vec2 VERTEX;

void main()
{
    gl_Position = vec4(VERTEX, 0.0, 1.0);
}
