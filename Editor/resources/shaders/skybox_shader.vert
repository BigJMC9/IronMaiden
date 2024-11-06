#version 450

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

const vec3 vertexs[36] = vec3[](
    // Front face (z = 1.0)
    vec3(-0.5, -0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(0.5, -0.5, 0.5),
    vec3(0.5, 0.5, 0.5), vec3(-0.5, -0.5, 0.5), vec3(-0.5, 0.5, 0.5),
    // Back face (z = -1.0)
    vec3(0.5, -0.5, -0.5), vec3(-0.5, 0.5, -0.5), vec3(-0.5, -0.5, -0.5), 
    vec3(-0.5, 0.5, -0.5), vec3(0.5, -0.5, -0.5), vec3(0.5, 0.5, -0.5), 
    // Top face (y = 1.0)
    vec3(-0.5, 0.5, -0.5), vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5), 
    vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, -0.5), vec3(0.5, 0.5, -0.5), 
    // Bottom face (y = -1.0)
    vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, -0.5), vec3(-0.5, -0.5, -0.5), 
    vec3(0.5, -0.5, -0.5), vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, 0.5), 
    // Right face (x = 1.0)
    vec3(0.5, -0.5, 0.5), vec3(0.5, 0.5, -0.5), vec3(0.5, -0.5, -0.5),
    vec3(0.5, 0.5, -0.5), vec3(0.5, -0.5, 0.5), vec3(0.5, 0.5, 0.5),
    // Left face (x = -1.0)
    vec3(-0.5, -0.5, -0.5), vec3(-0.5, 0.5, 0.5), vec3(-0.5, -0.5, 0.5),
    vec3(-0.5, 0.5, 0.5), vec3(-0.5, -0.5, -0.5), vec3(-0.5, 0.5, -0.5)
);

const vec2 uvs[36] = vec2[](
    // Front face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0), 
    // Back face
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, 0.0), 
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0), 
    // Top face
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0), 
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), 
    // Bottom face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 0.0), 
    // Right face
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), 
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0), 
    // Left face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0)
);


void main() {
	mat4 viewNoTranslation = mat4(mat3(ubo.view));
	vec4 pos = ubo.projection * viewNoTranslation * vec4(vertexs[gl_VertexIndex], 1.0);
	fragPos = vertexs[gl_VertexIndex].xyz;
	gl_Position = pos.xyww;
	fragUV = uvs[gl_VertexIndex];
}