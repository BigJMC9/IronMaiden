#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUno {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D diffuseMap;

const vec3 skyColorTop = vec3(0, 0.282, 0.729);
const vec3 skyColorHorizon = vec3(0.529, 0.808, 0.922);
const vec3 groundColor = vec3(0.5, 0.5, 0.5);

void main() {
	float t = clamp((fragPos.y + 1.0) / 2.0, 0.0, 1.0);
	vec3 skyColor;
	if (fragPos.y < 0.0)
	{
		skyColor = mix(skyColorTop, skyColorHorizon, t);
	}
	else
	{
		skyColor = mix(skyColorHorizon, groundColor, t);
	}

	vec3 newColor = texture(diffuseMap, fragUV).xyz;
	vec3 fragColor = newColor;

	outColor = vec4(fragColor, 1.0);
}