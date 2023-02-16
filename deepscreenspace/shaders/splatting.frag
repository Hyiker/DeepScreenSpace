#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "include/dss.glsl"
#include "include/math.glsl"
#include "include/positionNormal.glsl"
#include "include/sss.glsl"

layout(location = 0) out vec3 fragColor;

layout(location = 41) uniform float strength;

in Surfel geometrySurfel;

flat in float geometryInnerRadius;
flat in float geometryOuterRadius;
flat in float geometryMaxDistance;
flat in ivec4 geometrySubBufferBounds;

void main() {
    // Read position and normal of the pixel

    const vec3 pixelNormal =
        sampleNormal(gl_FragCoord.xy, gl_Layer) + vec3(0.01, 0.01, 0.01);
    const vec4 pixelPosition = samplePosition(gl_FragCoord.xy, gl_Layer);
    const float distanceToSurfel =
        length(vec3(pixelPosition) - geometrySurfel.position);

    if (pixelPosition.a > 0.0 && distanceToSurfel < geometryOuterRadius &&
        distanceToSurfel > geometryInnerRadius) {
        // FIXME
        fragColor =
            computeEffect(geometrySurfel,
                          SplatReceiver(vec3(pixelPosition), pixelNormal)) *
            strength;
    } else {
        fragColor = vec3(0.0);
    }
}
