#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;
layout (location = 4) in vec3 fragTangent;
layout (location = 5) in vec3 fragBiTangent;

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

layout(set = 0, binding = 0) uniform GlobalUbo {
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

float compareShadow(ShadowRecord record, vec2 uv, float depth) {
	vec2 halfTexel = vec2(record.options.z * 0.5);
	vec2 atlasUv = clamp(uv, record.atlasRect.xy + halfTexel,
		record.atlasRect.xy + record.atlasRect.zw - halfTexel);
	if (record.meta.x == 2) return texture(directionalShadowMap, vec3(atlasUv, depth));
	return record.meta.x == 0 ? texture(realtimeShadowMap, vec3(atlasUv, depth))
		: texture(bakedShadowMap, vec3(atlasUv, depth));
}

float rawShadowDepth(ShadowRecord record, vec2 uv) {
	vec2 halfTexel = vec2(record.options.z * 0.5);
	vec2 atlasUv = clamp(uv, record.atlasRect.xy + halfTexel,
		record.atlasRect.xy + record.atlasRect.zw - halfTexel);
	if (record.meta.x == 2) return texture(directionalDepthMap, atlasUv).r;
	return record.meta.x == 0 ? texture(realtimeDepthMap, atlasUv).r : texture(bakedDepthMap, atlasUv).r;
}

float shadowVisibility(ShadowRecord record, vec3 worldPosition, vec3 normal, vec3 directionToLight) {
	vec4 clip = record.viewProjection * vec4(worldPosition + normal * record.biasFilter.z, 1.0);
	if (clip.w <= 0.0) return 1.0;
	vec3 projected = clip.xyz / clip.w;
	vec2 uv = projected.xy * 0.5 + 0.5;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) ||
		projected.z <= 0.0 || projected.z >= 1.0) return 1.0;
	float bias = max(record.biasFilter.x * (1.0 - dot(normal, directionToLight)), record.biasFilter.x * 0.25);
	float receiver = projected.z - bias;
	vec2 atlasUv = record.atlasRect.xy + uv * record.atlasRect.zw;
	if (record.meta.y == 0) return compareShadow(record, atlasUv, receiver);
	float radius = record.meta.y == 1 ? 1.0 : 2.0;
	if (record.meta.y == 3) {
		float blockerSum = 0.0;
		float blockers = 0.0;
		for (int y = -2; y <= 2; ++y)
			for (int x = -2; x <= 2; ++x) {
				float sampleDepth = rawShadowDepth(record, atlasUv + vec2(x, y) * record.options.z);
				if (sampleDepth < receiver) { blockerSum += sampleDepth; blockers += 1.0; }
			}
		if (blockers == 0.0) return 1.0;
		float averageBlocker = blockerSum / blockers;
		radius = clamp((receiver - averageBlocker) * max(record.options.y, 0.5) * 120.0, 1.0, 7.0);
	}
	float visibility = 0.0;
	int kernel = record.meta.y == 1 ? 1 : 2;
	for (int y = -kernel; y <= kernel; ++y)
		for (int x = -kernel; x <= kernel; ++x)
			visibility += compareShadow(record, atlasUv + vec2(x, y) * radius * record.options.z / float(kernel), receiver);
	float side = float(kernel * 2 + 1);
	return visibility / (side * side);
}

int pointFace(vec3 direction) {
	vec3 magnitude = abs(direction);
	if (magnitude.x >= magnitude.y && magnitude.x >= magnitude.z) return direction.x >= 0.0 ? 0 : 1;
	if (magnitude.y >= magnitude.z) return direction.y >= 0.0 ? 2 : 3;
	return direction.z >= 0.0 ? 4 : 5;
}

float sampleLightRange(int first, int count, int type, int face, vec3 position, vec3 normal, vec3 toLight) {
	if (first <= 0 || count <= 0) return 1.0;
	int offset = type == 3 ? face : 0;
	if (type == 1 && count > 1) {
		float distanceToCamera = length(position - ubo.invView[3].xyz);
		for (int cascade = 0; cascade < count - 1; ++cascade)
			if (distanceToCamera > ubo.shadows[first - 1 + cascade].options.x) offset = cascade + 1;
	}
	return shadowVisibility(ubo.shadows[first - 1 + min(offset, count - 1)], position, normal, toLight);
}

float lightShadow(int type, int index, vec3 position, vec3 normal, vec3 toLight, vec3 lightPosition) {
	ivec4 range = type == 1 ? ubo.directionalShadowIndices[index]
		: type == 2 ? ubo.spotShadowIndices[index] : ubo.pointShadowIndices[index];
	int face = type == 3 ? pointFace(position - lightPosition) : 0;
	bool isStatic = push.normalMatrix[3].x > 0.5;
	bool receivesRealtime = push.normalMatrix[3].y > 0.5;
	float visibility = 1.0;
	if (range.y > 0 && (isStatic || (receivesRealtime && ubo.shadows[range.y - 1].meta.w == 1)))
		visibility = min(visibility, sampleLightRange(range.y, range.w, type, face, position, normal, toLight));
	if (receivesRealtime && range.x > 0)
		visibility = min(visibility, sampleLightRange(range.x, range.z, type, face, position, normal, toLight));
	return visibility;
}

layout (set = 1, binding = 0) uniform sampler2D diffuseMap;
layout (set = 1, binding = 1) uniform sampler2D normalMap;
layout (set = 1, binding = 2) uniform sampler2D ambientOcclusionMap;
layout (set = 1, binding = 3) uniform sampler2D glossMap;

void main() {
	vec3 normalMapColor = texture(normalMap, fragUv).xyz;
	vec3 decodedNormal = normalMapColor * 2.0 - 1.0;
	vec3 normalMapNormal = dot(decodedNormal, decodedNormal) > 0.0001
		? normalize(decodedNormal) : vec3(0.0, 0.0, 1.0);
	vec3 specularLight = vec3(0.0);
	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	mat3 TBN = mat3(fragTangent, fragBiTangent, fragNormalWorld);
	vec3 surfaceNormal = normalize(TBN * normalMapNormal);
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

	// Ambient Occlusion
    float ambientOcclusion = texture(ambientOcclusionMap, fragUv).r;
    diffuseLight *= ambientOcclusion;

	float glossiness = texture(glossMap, fragUv).r;

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
		float visibility = lightShadow(3, i, fragPosWorld, surfaceNormal, directionToLight, light.position.xyz);
		vec3 intensity = light.color.xyz * light.color.w * attenuation * visibility;

		diffuseLight += intensity * cosAngIncidence;

		// specular lighting
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 1.0 / max(glossiness, 0.02)); // higher values -> sharper highlight
		specularLight += intensity * blinnTerm;
	}

	for (int i = 0; i < ubo.numDirectionalLights; ++i) {
		DirectionalLight light = ubo.directionalLights[i];
		if (light.color.w <= 0.0) continue;
		vec3 toLight = normalize(-light.direction.xyz);
		float visibility = lightShadow(1, i, fragPosWorld, normalize(fragNormalWorld), toLight, vec3(0.0));
		vec3 intensity = light.color.rgb * light.color.w * visibility;
		diffuseLight += intensity * max(dot(surfaceNormal, toLight), 0.0);
		vec3 halfAngle = normalize(toLight + viewDirection);
		specularLight += intensity * pow(max(dot(surfaceNormal, halfAngle), 0.0), max(1.0, 1.0 / max(glossiness, 0.02)));
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
		float visibility = lightShadow(2, i, fragPosWorld, normalize(fragNormalWorld), toLight, light.position.xyz);
		vec3 intensity = light.color.rgb * light.color.w * coneFade * rangeFade * visibility / (1.0 + distanceSquared);
		diffuseLight += intensity * max(dot(surfaceNormal, toLight), 0.0);
		vec3 halfAngle = normalize(toLight + viewDirection);
		specularLight += intensity * pow(max(dot(surfaceNormal, halfAngle), 0.0), max(1.0, 1.0 / max(glossiness, 0.02)));
	}

	vec3 color = texture(diffuseMap, fragUv).xyz;
	outColor = vec4((diffuseLight * color) + (specularLight * fragColor), 1.0);
}
