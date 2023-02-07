#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "include/dss.glsl"
#include "include/math.glsl"
#include "include/positionNormal.glsl"
#include "include/sss.glsl"

layout(location = 0) out vec4 fragColor;

layout(location = 41) uniform float strength = 10.0;

in Surfel geometrySurfel;

in float geometryInnerRadius;
in float geometryOuterRadius;

void main() {
    // Read position and normal of the pixel
    const vec3 pixelNormal =
        sampleNormal(gl_FragCoord.xy, gl_Layer) + vec3(0.01, 0.01, 0.01);
    const vec4 pixelPosition = samplePosition(gl_FragCoord.xy, gl_Layer);
    const float distanceToSurfel =
        length(vec3(pixelPosition) - geometrySurfel.position);

    if (pixelPosition.a > 0.0 &&
        distanceToSurfel < geometryOuterRadius  // FIXME: this leads
                                                // invisibility of surfel effect
        && distanceToSurfel > geometryInnerRadius) {
        fragColor = vec4(
            computeEffect(geometrySurfel,
                          SplatReceiver(vec3(pixelPosition), pixelNormal)) *
                strength,
            1.0);
        fragColor = vec4(1.0);
    } else
        fragColor = vec4(0.0);
}
