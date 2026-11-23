#version 450
//written in GLSL version 450 (OpenGL Shading Language (Vulkan uses this language as well))

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // w is range
	vec4 color; // w is intensity
};

struct DirectionalLight { vec4 direction; vec4 color; };
struct SpotLight { vec4 position; vec4 direction; vec4 color; vec4 cone; };
struct ShadowRecord {
	mat4 viewProjection;
	vec4 atlasRect;
	vec4 biasFilter;
	vec4 options;
	ivec4 meta;
};

layout(set = 0, binding = 0) uniform GlobalUno {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
	int numDirectionalLights;
	int numSpotLights;
	int lightPadding;
	DirectionalLight directionalLights[2];
	SpotLight spotLights[8];
	ivec4 shadowCounts;
	ShadowRecord shadows[112];
	ivec4 pointShadowIndices[10];
	ivec4 directionalShadowIndices[2];
	ivec4 spotShadowIndices[8];
} ubo;

layout(set = 0, binding = 1) uniform sampler2DShadow realtimeShadowMap;
layout(set = 0, binding = 2) uniform sampler2DShadow bakedShadowMap;
layout(set = 0, binding = 3) uniform sampler2D realtimeDepthMap;
layout(set = 0, binding = 4) uniform sampler2D bakedDepthMap;
layout(set = 0, binding = 5) uniform sampler2DShadow directionalShadowMap;
layout(set = 0, binding = 6) uniform sampler2D directionalDepthMap;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

vec3 shadowGeometricNormal;
vec3 shadowSmoothNormal;
vec3 shadowReceiverPosition;
float shadowSmoothNormalMismatch;
float shadowTerminatorWidth;
vec3 shadowPositionDx;
vec3 shadowPositionDy;

vec2 shadowSampleUv(ShadowRecord record, vec2 uv) {
	vec2 halfTexel = vec2(record.options.z * 0.5);
	return clamp(uv, record.atlasRect.xy + halfTexel,
		record.atlasRect.xy + record.atlasRect.zw - halfTexel);
}

float shadowReceiverDepth(ShadowRecord record, vec2 uv, vec2 centerUv, float depth, vec2 depthSlope) {
	// The shadow sampler uses nearest filtering, so compare at the sampled texel's center.
	vec2 sampleUv = shadowSampleUv(record, uv);
	vec2 texelCenter = (floor(sampleUv / record.options.z) + vec2(0.5)) * record.options.z;
	return depth + dot(depthSlope, texelCenter - centerUv);
}

float compareShadow(ShadowRecord record, vec2 uv, vec2 centerUv, float depth, vec2 depthSlope) {
	vec2 atlasUv = shadowSampleUv(record, uv);
	float receiver = shadowReceiverDepth(record, uv, centerUv, depth, depthSlope);
	if (record.meta.x == 2) return texture(directionalShadowMap, vec3(atlasUv, receiver));
	return record.meta.x == 0 ? texture(realtimeShadowMap, vec3(atlasUv, receiver))
		: texture(bakedShadowMap, vec3(atlasUv, receiver));
}

float rawShadowDepth(ShadowRecord record, vec2 uv) {
	vec2 atlasUv = shadowSampleUv(record, uv);
	if (record.meta.x == 2) return texture(directionalDepthMap, atlasUv).r;
	return record.meta.x == 0 ? texture(realtimeDepthMap, atlasUv).r : texture(bakedDepthMap, atlasUv).r;
}

float shadowVisibility(ShadowRecord record, vec3 worldPosition, vec3 directionToLight) {
	vec4 unshiftedClip = record.viewProjection * vec4(worldPosition, 1.0);
	if (unshiftedClip.w <= 0.0) return 1.0;
	float facing = clamp(dot(shadowGeometricNormal, directionToLight), 0.0, 1.0);
	float grazing = 1.0 - facing;
	float worldTexel = record.biasFilter.w * (record.options.w >= 0.0 ? unshiftedClip.w : 1.0);
	// Correct the smooth-normal shadow terminator without shifting flat contact surfaces.
	float lightOffset = min(4.0 * worldTexel * grazing * grazing * shadowSmoothNormalMismatch, 0.05);
	vec4 clip = record.viewProjection * vec4(worldPosition +
		shadowGeometricNormal * record.biasFilter.z + directionToLight * lightOffset, 1.0);
	if (clip.w <= 0.0) return 1.0;
	vec3 projected = clip.xyz / clip.w;
	vec2 uv = projected.xy * 0.5 + 0.5;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) ||
		projected.z <= 0.0 || projected.z >= 1.0) return 1.0;
	float texelBias = record.options.w >= 0.0
		? record.options.w / max(abs(clip.w), 0.1) : -record.options.w;
	float bias = max(record.biasFilter.x * (1.0 - facing), record.biasFilter.x * 0.25);
	bias = max(bias, texelBias * 0.02);
	float receiver = projected.z - bias;
	vec2 atlasUv = record.atlasRect.xy + uv * record.atlasRect.zw;
	// Project the current triangle's receiver plane into the shadow atlas.
	// Each PCF tap must compare the depth at its own texel, not the center tap's depth.
	vec4 clipDx = record.viewProjection * vec4(shadowPositionDx, 0.0);
	vec4 clipDy = record.viewProjection * vec4(shadowPositionDy, 0.0);
	vec3 slopeX = (clipDx.xyz - projected * clipDx.w) / clip.w;
	vec3 slopeY = (clipDy.xyz - projected * clipDy.w) / clip.w;
	slopeX.xy *= record.atlasRect.zw * 0.5;
	slopeY.xy *= record.atlasRect.zw * 0.5;
	float determinant = slopeX.x * slopeY.y - slopeX.y * slopeY.x;
	vec2 depthSlope = vec2(0.0);
	float projectedArea = length(slopeX.xy) * length(slopeY.xy);
	if (projectedArea > 1e-20 && abs(determinant) > projectedArea * 1e-4)
		depthSlope = vec2(slopeX.z * slopeY.y - slopeY.z * slopeX.y,
			slopeY.z * slopeX.x - slopeX.z * slopeY.x) / determinant;
	if (record.meta.y == 0) return compareShadow(record, atlasUv, atlasUv, receiver, depthSlope);
	float radius = record.meta.y == 1 ? 1.0 : 2.0;
	if (record.meta.y == 3) {
		float blockerSum = 0.0;
		float blockers = 0.0;
		for (int y = -2; y <= 2; ++y)
			for (int x = -2; x <= 2; ++x) {
				vec2 sampleUv = atlasUv + vec2(x, y) * record.options.z;
				float sampleDepth = rawShadowDepth(record, sampleUv);
				if (sampleDepth < shadowReceiverDepth(record, sampleUv, atlasUv, receiver, depthSlope)) {
					blockerSum += sampleDepth;
					blockers += 1.0;
				}
			}
		if (blockers == 0.0) return 1.0;
		float averageBlocker = blockerSum / blockers;
		radius = clamp((receiver - averageBlocker) * max(record.options.y, 0.5) * 120.0, 1.0, 7.0);
	}
	float visibility = 0.0;
	int kernel = record.meta.y == 1 ? 1 : 2;
	for (int y = -kernel; y <= kernel; ++y)
		for (int x = -kernel; x <= kernel; ++x)
			visibility += compareShadow(record, atlasUv + vec2(x, y) * radius * record.options.z / float(kernel),
				atlasUv, receiver, depthSlope);
	float side = float(kernel * 2 + 1);
	return visibility / (side * side);
}

int pointFace(vec3 direction) {
	vec3 magnitude = abs(direction);
	if (magnitude.x >= magnitude.y && magnitude.x >= magnitude.z) return direction.x >= 0.0 ? 0 : 1;
	if (magnitude.y >= magnitude.z) return direction.y >= 0.0 ? 2 : 3;
	return direction.z >= 0.0 ? 4 : 5;
}

float sampleLightRange(int first, int count, int type, int face, vec3 position, vec3 toLight) {
	if (first <= 0 || count <= 0) return 1.0;
	int offset = type == 3 ? face : 0;
	if (type == 1 && count > 1) {
		float distanceToCamera = length(position - ubo.invView[3].xyz);
		for (int cascade = 0; cascade < count - 1; ++cascade)
			if (distanceToCamera > ubo.shadows[first - 1 + cascade].options.x) offset = cascade + 1;
	}
	return shadowVisibility(ubo.shadows[first - 1 + min(offset, count - 1)], position, toLight);
}

float lightShadow(int type, int index, vec3 toLight, vec3 lightPosition) {
	ivec4 range = type == 1 ? ubo.directionalShadowIndices[index]
		: type == 2 ? ubo.spotShadowIndices[index] : ubo.pointShadowIndices[index];
	int face = type == 3 ? pointFace(shadowReceiverPosition - lightPosition) : 0;
	bool isStatic = push.normalMatrix[3].x > 0.5;
	bool receivesRealtime = push.normalMatrix[3].y > 0.5;
	float visibility = 1.0;
	if (range.y > 0 && (isStatic || (receivesRealtime && ubo.shadows[range.y - 1].meta.w == 1)))
		visibility = min(visibility, sampleLightRange(range.y, range.w, type, face, shadowReceiverPosition, toLight));
	if (receivesRealtime && range.x > 0)
		visibility = min(visibility, sampleLightRange(range.x, range.z, type, face, shadowReceiverPosition, toLight));
	float smoothFacing = dot(shadowSmoothNormal, toLight);
	if (shadowTerminatorWidth > 0.001 && visibility < 1.0 && smoothFacing > 0.0) {
		// A smoothed surface can face the light where its flat triangle still self-shadows.
		// Fade that last sliver toward the smooth lighting terminator; flat contact surfaces are unchanged.
		float geometricFacing = max(dot(shadowGeometricNormal, toLight), 0.0);
		float shadowWeight = max(smoothstep(0.0, shadowTerminatorWidth, smoothFacing),
			smoothstep(0.0, shadowTerminatorWidth, geometricFacing));
		visibility = mix(1.0, visibility, shadowWeight);
	}
	return visibility;
}

void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);
	shadowSmoothNormal = surfaceNormal;
	vec3 geometricCross = cross(dFdx(fragPosWorld), dFdy(fragPosWorld));
	shadowGeometricNormal = dot(geometricCross, geometricCross) > 1e-12
		? normalize(geometricCross) : surfaceNormal;
	if (dot(shadowGeometricNormal, surfaceNormal) < 0.0) shadowGeometricNormal = -shadowGeometricNormal;
	float normalDifference = max(1.0 - dot(shadowGeometricNormal, surfaceNormal), 0.0);
	bool spherePrimitive = push.normalMatrix[3].w > 1.5;
	shadowReceiverPosition = fragPosWorld;
	if (spherePrimitive) {
		// The built-in sphere is a unit sphere before its model transform. Its smooth
		// surface encloses the polygon mesh, so lit points cannot shadow themselves.
		vec3 localPosition = transpose(mat3(push.normalMatrix)) *
			(fragPosWorld - push.modelMatrix[3].xyz);
		float radius = length(localPosition);
		if (radius > 1e-5)
			shadowReceiverPosition = (push.modelMatrix * vec4(localPosition / radius, 1.0)).xyz;
	}
	shadowPositionDx = dFdx(shadowReceiverPosition);
	shadowPositionDy = dFdy(shadowReceiverPosition);
	shadowSmoothNormalMismatch = spherePrimitive ? 0.0 : clamp(normalDifference * 16.0, 0.0, 1.0);
	float localAngle = sqrt(2.0 * normalDifference);
	shadowTerminatorWidth = spherePrimitive ? 0.0 :
		min(2.5 * push.normalMatrix[3].z, 0.35) * smoothstep(0.0, 0.02, localAngle);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPosWorld;
		float distanceSquared = max(dot(directionToLight, directionToLight), 0.01);
		float distanceToLight = sqrt(distanceSquared);
		if (distanceToLight >= light.position.w || light.color.w <= 0.0) continue;
		float rangeFade = pow(clamp(1.0 - distanceToLight / max(light.position.w, 0.01), 0.0, 1.0), 2.0);
		float attenuation = rangeFade / (1.0 + distanceSquared);
		directionToLight = normalize(directionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
		float visibility = lightShadow(3, i, directionToLight, light.position.xyz);
		vec3 intensity = light.color.xyz * light.color.w * attenuation * visibility;

		diffuseLight += intensity * cosAngIncidence;

		//specular lighting
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 32.0); // higher values -> sharper highlight
		specularLight += intensity * blinnTerm;
	}

	for (int i = 0; i < ubo.numDirectionalLights; ++i) {
		DirectionalLight light = ubo.directionalLights[i];
		if (light.color.w <= 0.0) continue;
		vec3 toLight = normalize(-light.direction.xyz);
		float visibility = lightShadow(1, i, toLight, vec3(0.0));
		vec3 intensity = light.color.rgb * light.color.w * visibility;
		diffuseLight += intensity * max(dot(surfaceNormal, toLight), 0.0);
		vec3 halfAngle = normalize(toLight + viewDirection);
		specularLight += intensity * pow(max(dot(surfaceNormal, halfAngle), 0.0), 32.0);
	}

	for (int i = 0; i < ubo.numSpotLights; ++i) {
		SpotLight light = ubo.spotLights[i];
		vec3 offset = light.position.xyz - fragPosWorld;
		float distanceSquared = max(dot(offset, offset), 0.01);
		float distanceToLight = sqrt(distanceSquared);
		if (distanceToLight >= light.cone.z || light.color.w <= 0.0) continue;
		vec3 toLight = offset / distanceToLight;
		float coneCosine = dot(-toLight, normalize(light.direction.xyz));
		if (coneCosine <= light.cone.y) continue;
		float coneFade = smoothstep(light.cone.y, light.cone.x, coneCosine);
		float rangeFade = pow(clamp(1.0 - distanceToLight / max(light.cone.z, 0.01), 0.0, 1.0), 2.0);
		float visibility = lightShadow(2, i, toLight, light.position.xyz);
		vec3 intensity = light.color.rgb * light.color.w * coneFade * rangeFade * visibility / (1.0 + distanceSquared);
		diffuseLight += intensity * max(dot(surfaceNormal, toLight), 0.0);
		vec3 halfAngle = normalize(toLight + viewDirection);
		specularLight += intensity * pow(max(dot(surfaceNormal, halfAngle), 0.0), 32.0);
	}

	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}
