#version 450
#extension GL_ARB_derivative_control : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
//layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUv;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBiTangent;

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

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	//Calculate Tangent
	//vec3 edge0 = dFdx(position);
	//vec3 edge1 = dFdy(position);
	//vec2 deltaUV0 = dFdx(uv);
	//vec2 deltaUV1 = dFdy(uv);

	//float f = 1.0 / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);
	//vec3 tangentCandidate = normalize(f * (deltaUV1.y * edge0 - deltaUV0.y * edge1));
	
	//fragTangent = tangentCandidate;

	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projection * ubo.view * positionWorld;
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
	fragUv = uv;

	vec3 T, B;
	T = normalize(vec3(push.modelMatrix * vec4(cross(normal, vec3(0.0, 0.0, 1.0)), 0.0)));
	B = normalize(vec3(push.modelMatrix * vec4(cross(fragNormalWorld, T), 0.0)));
	fragTangent = T;
	fragBiTangent = B;
	//fragTangent = tangent;
}