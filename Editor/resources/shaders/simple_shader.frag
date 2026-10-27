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
	mat4 shadowViewProjection;
	vec4 shadowParams;
	ivec4 shadowInfo;
} ubo;

layout(set = 0, binding = 1) uniform sampler2DShadow realtimeShadowMap;
layout(set = 0, binding = 2) uniform sampler2DShadow bakedShadowMap;

float shadowVisibility(vec3 worldPosition, vec3 normal, vec3 directionToLight) {
	if (ubo.shadowInfo.x == 0) return 1.0;
	vec4 clip = ubo.shadowViewProjection * vec4(worldPosition, 1.0);
	if (clip.w <= 0.0) return 1.0;
	vec3 projected = clip.xyz / clip.w;
	vec2 uv = projected.xy * 0.5 + 0.5;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) ||
		projected.z <= 0.0 || projected.z >= 1.0) return 1.0;
	float bias = max(ubo.shadowParams.x * (1.0 - dot(normal, directionToLight)), ubo.shadowParams.x * 0.25);
	vec3 samplePos = vec3(uv, projected.z - bias);
	return ubo.shadowInfo.x == 2
		? texture(bakedShadowMap, samplePos) : texture(realtimeShadowMap, samplePos);
}

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPosWorld;
		float distanceSquared = max(dot(directionToLight, directionToLight), 0.01);
		float distanceToLight = sqrt(distanceSquared);
		float rangeFade = pow(clamp(1.0 - distanceToLight / max(light.position.w, 0.01), 0.0, 1.0), 2.0);
		float attenuation = rangeFade / (1.0 + distanceSquared);
		directionToLight = normalize(directionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

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
		vec3 toLight = normalize(-light.direction.xyz);
		float visibility = ubo.shadowInfo.y == 1 && ubo.shadowInfo.z == i
			? shadowVisibility(fragPosWorld, surfaceNormal, toLight) : 1.0;
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
		vec3 toLight = offset / distanceToLight;
		float coneCosine = dot(-toLight, normalize(light.direction.xyz));
		float coneFade = smoothstep(light.cone.y, light.cone.x, coneCosine);
		float rangeFade = pow(clamp(1.0 - distanceToLight / max(light.cone.z, 0.01), 0.0, 1.0), 2.0);
		float visibility = ubo.shadowInfo.y == 2 && ubo.shadowInfo.z == i
			? shadowVisibility(fragPosWorld, surfaceNormal, toLight) : 1.0;
		vec3 intensity = light.color.rgb * light.color.w * coneFade * rangeFade * visibility / (1.0 + distanceSquared);
		diffuseLight += intensity * max(dot(surfaceNormal, toLight), 0.0);
		vec3 halfAngle = normalize(toLight + viewDirection);
		specularLight += intensity * pow(max(dot(surfaceNormal, halfAngle), 0.0), 32.0);
	}

	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}
