struct SurfelData {
    vec4 position;
    vec3 normal;
    float radius;
};

const int N_SURFELS_MAX = 2000000;
layout(binding = 0) uniform atomic_uint surfelCounter;
layout(std430, binding = 10) buffer surfelSSBO { SurfelData surfels[]; };