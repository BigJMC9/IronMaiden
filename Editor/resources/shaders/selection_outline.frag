#version 450

layout(push_constant) uniform OutlinePush {
    mat4 model;
    vec4 settings;  // viewportWidth, viewportHeight, outlineWidthPixels, opacity
} outline;

layout(location = 0) out vec4 outColor;

void main() {
    // Linear orange; the existing sRGB render target converts this for display.
    outColor = vec4(1.0, 0.27, 0.018, outline.settings.w);
}
