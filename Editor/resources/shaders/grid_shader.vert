#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_debug_printf : enable

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

//Has to be same order as GlobalUno struct or data will be in the wrong position or some weird stuff
layout(set = 0, binding = 0) uniform GlobalUno {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	float nearPlane;
    float farPlane;
} push;

layout(location = 0) out float near;
layout(location = 1) out float far;
layout(location = 2) out vec3 nearPoint;
layout(location = 3) out vec3 farPoint;
layout(location = 4) out mat4 fragView;
layout(location = 8) out mat4 fragProj;


// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 invView) {
    vec4 clipSpacePos = vec4(x, y, z, 1.0);
    vec4 worldSpacePos = invView * clipSpacePos;
    return worldSpacePos.xyz / worldSpacePos.w;
}

// normal vertice projection
void main() {
     // Precompute the inverse of the view-projection matrix for unprojection
    mat4 invViewProj = inverse(ubo.projection * ubo.view);
    
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, invViewProj); // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, invViewProj); // unprojecting on the far plane
    near = push.nearPlane;
    far = push.farPlane;

    // Pass the view and projection matrices as outputs for later stages
    fragView = ubo.view;
    fragProj = ubo.projection;
    
    // Set the vertex position directly in clip space
    gl_Position = vec4(p, 1.0);
}