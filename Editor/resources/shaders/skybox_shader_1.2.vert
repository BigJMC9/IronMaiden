#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragPos;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

//Has to be same order as GlobalUbo struct or data will be in the wrong position or some weird stuff
layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

void main() {

    mat4 noTranslationView = mat4(mat3(ubo.view));
    vec4 pos = ubo.projection * noTranslationView * vec4(position, 1.0);
    fragPos = pos.xyz;
    fragUV = uv;

    // Set the position in clip space
    gl_Position = pos;
}