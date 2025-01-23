#version 460

layout (push_constant) uniform PushConstants {
	layout (offset = 16) mat4 translationMatrix;
} pushConstants;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTransform0;
layout(location = 4) in vec4 inTransform1;
layout(location = 5) in vec4 inTransform2;
layout(location = 6) in vec4 inTransform3;
layout(location = 7) in uint inTextureIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;
layout (location = 2) out float outShading;

void main() {
	gl_Position = pushConstants.translationMatrix * mat4(inTransform0, inTransform1, inTransform2, inTransform3) * vec4(inVertex, 1.0);
	outUV = inUV;
	outTextureIndex = inTextureIndex;

	float shading = dot(inNormal, vec3(0, 0, 1));
	outShading = shading == 1 ? 1 : max(0.6, 1 - pow(2, -10 * shading));
}