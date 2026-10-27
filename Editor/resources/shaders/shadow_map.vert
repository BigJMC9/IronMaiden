#version 450

layout(location = 0) in vec3 position;

layout(push_constant) uniform ShadowPush {
    mat4 lightViewProjection;
    mat4 model;
} push;

void main() {
    gl_Position = push.lightViewProjection * push.model * vec4(position, 1.0);
}
