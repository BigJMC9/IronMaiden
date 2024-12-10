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
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D diffuseMap;
layout (set = 1, binding = 1) uniform sampler2D normalMap;
layout (set = 1, binding = 2) uniform sampler2D ambientOcclusionMap;
layout (set = 1, binding = 3) uniform sampler2D glossMap;

layout(push_constant) uniform Push {
	mat4 modelMatrix; //transform
	mat4 normalMatrix; //normal
} push;

void main() {
	vec3 normalMapColor = texture(normalMap, fragUv).xyz;
	vec3 normalMapNormal = normalize(normalMapColor);
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
		float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
		directionToLight = normalize(directionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngIncidence;

		// specular lighting
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 1.0 / glossiness); // higher values -> sharper highlight
		specularLight += intensity * blinnTerm;
	}

	vec3 color = texture(diffuseMap, fragUv).xyz;
	const vec3 SUN_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
	outColor = vec4((diffuseLight * color) + (specularLight * fragColor), 1.0);
}