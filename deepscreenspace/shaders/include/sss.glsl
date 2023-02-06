#ifndef DEEPSCREENSPACE_SHADERS_INCLUDE_SSS_GLSL
#define DEEPSCREENSPACE_SHADERS_INCLUDE_SSS_GLSL

#extension GL_GOOGLE_include_directive : enable
#include "loo/lights.glsl"
#include "loo/simple_material.glsl"
#include "math.glsl"
#include "surfel.glsl"
struct SplatReceiver {
    vec3 position;
    vec3 normal;
};

layout(location = 80) uniform float eta;
layout(location = 81) uniform float clampDistance;
layout(location = 82) uniform sampler2DArray coefficientTexture2DArray;
layout(location = 83) uniform float sizeFactor;

vec3 computeIrradiance(in vec3 position, in vec3 normal) {
    vec3 irradiance = vec3(0.0);
    for (int i = 0; i < nLights; i++) {
        ShaderLight light = lights[i];
        float distance = 1.0;
        vec3 L;
        switch (light.type) {
            case LIGHT_TYPE_DIRECTIONAL:
                L = normalize(-lights[0].direction.xyz);
                break;
            default:
                continue;
        }
        float attenuation = light.intensity / (distance * distance);
        vec3 Ld =
            light.color.rgb * vec3(attenuation * max(dot(L, normal), 0.0));
        irradiance += Ld;
    }
    return irradiance;
}

//////////////////////////
// Per-surfel computations
//////////////////////////

Surfel initSurfel(vec3 position, vec3 normal, float radius
                  //  ,int materialId
) {
    // vec3 sigma_a =
    //     texelFetch(coefficientTexture2DArray, ivec3(0, 0, materialId * 2), 0)
    //         .rgb;
    // vec3 sigma_s_prime = texelFetch(coefficientTexture2DArray,
    //                                 ivec3(0, 0, materialId * 2 + 1), 0)
    //                          .rgb;
    vec3 sigma_a = vec3(0.1);
    vec3 sigma_s_prime = vec3(0.001);
    return Surfel(position, normal, radius, computeIrradiance(position, normal),
                  sigma_a, sigma_s_prime);
}

float maximumDistance(const in Surfel surfel, const in float epsilon) {
    return surfel.radius * sqrt(1.0 / epsilon) *
           sqrt(max(surfel.light.x, max(surfel.light.y, surfel.light.z)));
}

////////////////////////////
// Per-fragment computations
////////////////////////////

vec3 computeRadiantExitance(const in vec3 x_in, const in vec3 x_out,
                            const in float sampleArea,
                            const in vec3 sigma_s_prime,
                            const in vec3 sigma_a) {
    vec3 v = x_out - x_in;  // Vector from receiving point to surfel center
    float r_square =
        dot(v, v);  // Squared distance between surfel and exiting point

    r_square *= sizeFactor * sizeFactor;
    r_square = max(clampDistance * clampDistance, r_square);

    // vec3 sigma_s_prime = (1.0 - g) * sigma_s;
    // // Reduced scattering coefficient

    vec3 sigma_t_prime =
        sigma_s_prime + sigma_a;  // Reduced extinction coefficient
    vec3 sigma_tr =
        sqrt(3.0 * sigma_a *
             sigma_t_prime);  // Effective transport extinction coefficient

    vec3 l_u = 1.0 / sigma_t_prime;  // Mean-free path

    float F_dr = -1.44 / (eta * eta) + 0.71 / eta + 0.668 +
                 0.0636 * eta;  // Diffuse Fresnel term
    float A = (1.0 + F_dr) / (1.0 - F_dr);

    vec3 z_r = l_u;  // Distance of inner dipole light to surface
    vec3 z_v =
        l_u * (1.0 + 1.3333 * A);  // Distance of outer dipole light to surface

    vec3 d_r = sqrt(r_square + z_r * z_r);  // Distance to real light source
    vec3 d_v = sqrt(r_square + z_v * z_v);  // Distance to virtual light source

    vec3 C_1 = z_r * (sigma_tr + 1.0 / d_r);
    vec3 C_2 = z_v * (sigma_tr + 1.0 / d_v);

    return (1.0 - F_dr) *
           (C_1 * exp(-sigma_tr * d_r) / (d_r * d_r) +
            C_2 * exp(-sigma_tr * d_v) / (d_v * d_v)) *
           0.25 * (1.0 / PI) * sampleArea;
}

float radianceFactor(const in vec3 direction) {
    // float F_t =
    // // Fresnel transmittance float F_dr = -1.44f/(eta*eta) + 0.71f/eta +
    // 0.668f
    // + 0.0636f*eta;	// Diffuse Fresnel term return F_t / (F_dr*M_PI);
    return 1.0 / PI;
}

vec3 computeEffect(const in Surfel surfel, const in SplatReceiver receiver) {
    vec3 sigma_a = surfel.sigma_a;
    vec3 sigma_s_prime = surfel.sigma_s_prime;

    return radianceFactor(vec3(0.0, 0.0, 0.0)) *
           computeRadiantExitance(surfel.position, receiver.position,
                                  surfel.radius * surfel.radius * PI,
                                  sigma_s_prime, sigma_a) *
           surfel.light;  // * texelFetch(diffuseTexture2DArray,
                          // ivec3(gl_FragCoord.xy, gl_Layer), 0).rgb;
}

#endif /* DEEPSCREENSPACE_SHADERS_INCLUDE_SSS_GLSL */
