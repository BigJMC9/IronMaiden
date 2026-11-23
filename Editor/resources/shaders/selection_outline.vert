#version 450

// A screen-space expanded hull. No scene-depth sampling is needed: the fixed
// Vulkan depth test separates the visible and occluded pixels in two draws.
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;

// Matches the first two fields of IronMaiden's GlobalUbo.
layout(set = 0, binding = 0) uniform GlobalCamera {
    mat4 projection;
    mat4 view;
} camera;

layout(push_constant) uniform OutlinePush {
    mat4 model;
    vec4 settings;  // viewportWidth, viewportHeight, outlineWidthPixels, opacity
} outline;

void main() {
    mat4 vp = camera.projection * camera.view;
    vec4 clip = vp * outline.model * vec4(inPosition, 1.0);

    if (outline.settings.z > 0.0 && clip.w > 0.0001) {
        // Project a small normal displacement to get a screen-space expansion
        // direction. This preserves a roughly constant pixel width with zoom.
        mat3 normalMatrix = transpose(inverse(mat3(outline.model)));
        vec3 worldNormal = normalMatrix * inNormal;
        vec2 dirPixels = vec2(0.0);
        float normalLength = length(worldNormal);
        if (normalLength > 0.00001) {
            vec4 worldPosition = outline.model * vec4(inPosition, 1.0);
            vec4 normalClip = vp * (worldPosition + vec4(worldNormal * (0.1 / normalLength), 0.0));
            if (normalClip.w > 0.0001) {
                dirPixels = (normalClip.xy / normalClip.w - clip.xy / clip.w)
                          * outline.settings.xy;
            }
        }

        // Vertex normals perpendicular to the image plane can project to zero.
        // Use radial expansion in that case, rather than leaving a gap.
        if (length(dirPixels) < 0.00001) {
            vec4 originClip = vp * outline.model * vec4(0.0, 0.0, 0.0, 1.0);
            if (originClip.w > 0.0001) {
                dirPixels = (clip.xy / clip.w - originClip.xy / originClip.w)
                          * outline.settings.xy;
            }
        }

        float directionLength = length(dirPixels);
        if (directionLength > 0.00001) {
            vec2 offsetNdc = normalize(dirPixels) *
                (2.0 * outline.settings.z / outline.settings.xy);
            clip.xy += offsetNdc * clip.w;
        }
    }

    gl_Position = clip;
}
