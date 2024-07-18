#version 450

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragPos;
layout(location = 4) out mat4 fragView;
layout(location = 8) out mat4 fragProj;

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

vec3 plane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec2 uvs[6] = vec2[](
    vec2(1, 1), vec2(0, 1), vec2(0, 0),
    vec2(0, 0), vec2(1, 0), vec2(1, 1)
);


void main() {
	fragPos = plane[gl_VertexIndex].xyz;
	gl_Position = vec4(plane[gl_VertexIndex], 1.0);
	fragUV = uvs[gl_VertexIndex];
}

