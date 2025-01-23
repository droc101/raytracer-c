#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint skyVertexCount;
	uint skyTextureIndex;
	uint shadowTextureIndex;
} pushConstants;

layout (binding = 0) uniform sampler2D textureSampler[];

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in uint inTextureIndex;
layout (location = 2) flat in float inShading;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV) * vec4(inShading, inShading, inShading, 1);
	if (outColor.a < 0.5) discard;
	outColor.a = 1.0;
	if (inTextureIndex == pushConstants.shadowTextureIndex) {
		outColor.a = 0.5;
	}
}
