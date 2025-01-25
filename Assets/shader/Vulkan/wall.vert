#version 460

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

layout (location = 0) in vec3 inShadowVertex;
layout (location = 1) in vec3 inWallVertex;
layout (location = 2) in vec2 inUV;
layout (location = 3) in uint inTextureIndex;
layout (location = 4) in float inWallAngle;

layout (location = 0) out vec2 outUV;
layout (location = 1) flat out uint outTextureIndex;
layout (location = 2) flat out vec4 outColor;

const vec2 shadowUV[] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 1.0)
);

void main() {
	outColor = vec4(unpackUnorm4x8(pushConstants.fogColor).bgr, 1);
	if (gl_InstanceIndex == 0x57414C4C) {
		// Walls
		gl_Position = pushConstants.translationMatrix * vec4(inWallVertex, 1.0);
		outUV = inUV;
		outTextureIndex = inTextureIndex;
		outColor.a = max(0.6, min(1, abs(cos(pushConstants.yaw - inWallAngle))));
		return;
	}
	if (gl_InstanceIndex == 0x53484457) {
		// Shadows
		gl_Position = pushConstants.translationMatrix * vec4(inShadowVertex, 1.0);
		outUV = shadowUV[gl_VertexIndex % 4];
		outTextureIndex = pushConstants.shadowTextureIndex;
		return;
	}
	if (inTextureIndex == pushConstants.skyTextureIndex)
	{
		// Sky
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV;
		outTextureIndex = inTextureIndex;
		return;
	} else {
		// Floor and Ceiling
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV + pushConstants.playerPosition;
		outTextureIndex = inTextureIndex;
		if (pushConstants.skyVertexCount == 0 && gl_VertexIndex < 8 && 4 <= gl_VertexIndex) {
			// Ceiling
			outColor.a = 0.8;
		}
		return;
	}
}