#version 460
#extension GL_EXT_nonuniform_qualifier: enable

layout (push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint skyVertexCount;
	uint skyTextureIndex;

	uint shadowTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
} pushConstants;

layout (binding = 0) uniform sampler2D textureSampler[];

layout (location = 0) in vec2 inUV;
layout (location = 1) flat in uint inTextureIndex;
layout (location = 2) flat in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() {
	gl_FragDepth = gl_FragCoord.z;
	float fog_factor = clamp((gl_FragCoord.z / gl_FragCoord.w - pushConstants.fogStart) / (pushConstants.fogEnd - pushConstants.fogStart), 0.0, 1.0);
	outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	outColor *= inColor.a;
	outColor.rgb = mix(outColor.rgb, inColor.rgb, fog_factor);
	if (outColor.a < 0.5) discard;
	outColor.a = 1.0;
	if (inTextureIndex == pushConstants.shadowTextureIndex) {
		outColor.a = 0.5;
		return;
	}
	if (inTextureIndex == pushConstants.skyTextureIndex) {
		gl_FragDepth = 0.9999999;
		return;
	}
}
