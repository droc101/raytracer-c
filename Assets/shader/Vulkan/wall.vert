#version 460

layout(push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint skyVertexCount;
	uint skyTextureIndex;
	uint shadowTextureIndex;
} pushConstants;

layout (location = 0) in vec3 inShadowVertex;
layout (location = 1) in vec3 inWallVertex;
layout (location = 2) in vec2 inUV;
layout (location = 3) in uint inTextureIndex;
layout (location = 4) in float inWallAngle;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;
layout (location = 2) flat out float outShading;

const vec2 shadowUV[] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 1.0)
);

void main() {
	if (gl_InstanceIndex == 0x57414C4C) {
		gl_Position = pushConstants.translationMatrix * vec4(inWallVertex, 1.0);
		outUV = inUV;
		outTextureIndex = inTextureIndex;
		outShading = max(0.6, min(1, abs(cos(pushConstants.yaw - inWallAngle))));
		return;
	}
	if (gl_InstanceIndex == 0x53484457) {
		// Shadows
		gl_Position = pushConstants.translationMatrix * vec4(inShadowVertex, 1.0);
		outUV = shadowUV[gl_VertexIndex % 4];
		outTextureIndex = pushConstants.shadowTextureIndex;
		outShading = 1;
		return;
	}
	if ((pushConstants.skyVertexCount == 0 && gl_VertexIndex < 8) ||
	(gl_VertexIndex < pushConstants.skyVertexCount + 4 && pushConstants.skyVertexCount <= gl_VertexIndex))
	{
		// Floor and Ceiling
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV + pushConstants.playerPosition;
		outTextureIndex = inTextureIndex;
		if (pushConstants.skyVertexCount == 0 && gl_VertexIndex < 8 && 4 <= gl_VertexIndex) { // Ceiling
																							  outShading = 0.8;
		} else { // Floor
				 outShading = 1;
		}
		return;
	} else if (gl_VertexIndex < pushConstants.skyVertexCount + 4)
	{
		// Sky
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV;
		outTextureIndex = pushConstants.skyTextureIndex;
		outShading = 1;
		return;
	}
}