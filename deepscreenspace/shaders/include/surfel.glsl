#ifndef DEEPSCREENSPACE_SHADERS_INCLUDE_SURFEL_GLSL
#define DEEPSCREENSPACE_SHADERS_INCLUDE_SURFEL_GLSL

struct SurfelData {
    vec4 position;
    vec3 normal;
    float radius;
};

struct Surfel {
    vec3 position;
    vec3 normal;
    float radius;
    vec3 light;
    vec3 sigma_a;
    vec3 sigma_s_prime;
    // int materialId;
};

#endif /* DEEPSCREENSPACE_SHADERS_INCLUDE_SURFEL_GLSL */
