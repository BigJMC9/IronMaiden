#version 450

layout(location = 0) in float near;
layout(location = 1) in float far;
layout(location = 2) in vec3 nearPoint;
layout(location = 3) in vec3 farPoint;
layout(location = 4) in mat4 fragView;
layout(location = 8) in mat4 fragProj;
layout(location = 0) out vec4 outColor;

vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 derivative = max(fwidth(fragPos3D.xz) * scale, vec2(0.0001));
    vec2 grid = abs(fract((fragPos3D.xz * scale) - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y / scale, 1);
    float minimumx = min(derivative.x / scale, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -1.0 * minimumx && fragPos3D.x < 1.0 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -1.0 * minimumz && fragPos3D.z < 1.0 * minimumz)
        color.x = 1.0;
    return color;
}
float computeDepth(vec3 pos) {
    vec4 clip_space_pos = fragProj * fragView * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}
float fadingFactor(vec3 pos, vec3 cameraPos)
{
    float dist = distance(cameraPos, pos);
    float nearFade = smoothstep(1.0, 8.0, dist);
    float farFade = 1.0 - smoothstep(35.0, 110.0, dist);
    float grazingAngle = abs((pos.y - cameraPos.y) / max(dist, 0.001));
    float horizonFade = smoothstep(0.02, 0.12, grazingAngle);
    return nearFade * farFade * horizonFade;
}
void main() {

    float _GridDiv = 10;
    float _GridBias = 0.5;
    

    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    if (t <= 0.0 || t >= 1.0)
        discard;
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    mat4 invView = inverse(fragView);
    vec3 cameraPosWorld = invView[3].xyz;
    float gridDiv = max(round(_GridDiv), 2.0);
    float viewLength = distance(cameraPosWorld.y, fragPos3D.y);
    float logLength = log(max(viewLength, 0.01))/log(gridDiv) - _GridBias;

    float logA = floor(logLength);
    float logB = logA + 1.0;
    float blendFactor = fract(logLength);
    float scaleA = 1 / round(pow(gridDiv, logA));
    float scaleB = 1 / round(pow(gridDiv, logB));

    vec4 gridSmall = grid(fragPos3D, scaleA, true);
    vec4 gridBig = grid(fragPos3D, scaleB, true);

    float smallCoverage = (1.0 - blendFactor) * gridSmall.a;
    float bigCoverage = blendFactor * gridBig.a;
    float coverage = max(smallCoverage, bigCoverage);
    vec3 lineColor = smallCoverage >= bigCoverage ? gridSmall.rgb : gridBig.rgb;
    outColor = vec4(lineColor, clamp(coverage * fadingFactor(fragPos3D, cameraPosWorld), 0.0, 0.5));
}
