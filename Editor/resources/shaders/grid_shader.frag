#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in float near;
layout(location = 1) in float far;
layout(location = 2) in vec3 nearPoint;
layout(location = 3) in vec3 farPoint;
layout(location = 4) in mat4 fragView;
layout(location = 8) in mat4 fragProj;
layout(location = 0) out vec4 outColor;

vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 derivative = fwidth(fragPos3D.xz) * scale;
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
    float threshold = 1 - (dist / far);
    return min(1, threshold * 10);
}
void main() {

    float _GridDiv = 10;
    float _GridBias = 0.5;
    

    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    mat4 invView = inverse(fragView);
    vec3 cameraPosWorld = invView[3].xyz;
    float gridDiv = max(round(_GridDiv), 2.0);
    float viewLength = distance(cameraPosWorld.y, fragPos3D.y);
    float logLength = log(viewLength)/log(gridDiv) - _GridBias;

    float logA = floor(logLength);
    float logB = logA + 1.0;
    float blendFactor = fract(logLength);
    float scaleA = 1 / round(pow(gridDiv, logA));
    float scaleB = 1 / round(pow(gridDiv, logB));

    /*if (gl_FragCoord.x < 523 && gl_FragCoord.y < 961 && gl_FragCoord.x > 521 && gl_FragCoord.y > 959)
    {
        debugPrintfEXT("FragCoord: vec2(%f, %f)\n\t fragPos3D vec2(%f, %f)\n\t viewLength: %f\n\t logLength: %f\n\t logA: %f\n\t logB: %f\n\t blendFactor: %f\n\t scaleA: %f\n\t scaleB: %f\n",
        gl_FragCoord.x, gl_FragCoord.y,
        fragPos3D.x, fragPos3D.z,
        viewLength,
        logLength,
        logA,
        logB,
        blendFactor,
        scaleA,
        scaleB);
    }*/

    vec4 gridSmall = grid(fragPos3D, scaleA, true);
    vec4 gridBig = grid(fragPos3D, scaleB, true);

    //vec3 dist = distance(cameraPosWorld, fragPos3D);

    outColor = max(vec4(gridBig.xyz, (blendFactor * gridBig.a)), vec4(gridSmall.xyz, ((1 - blendFactor) * gridSmall.a))) * float(t > 0);
    outColor.a *= fadingFactor(fragPos3D, cameraPosWorld);
}