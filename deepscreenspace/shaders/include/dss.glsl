#ifndef DEEPSCREENSPACE_SHADERS_INCLUDE_DSS_GLSL
#define DEEPSCREENSPACE_SHADERS_INCLUDE_DSS_GLSL
#extension GL_GOOGLE_include_directive : enable
#include "surfel.glsl"
const int N_SURFELS_MAX = 20000000;
layout(binding = 0) uniform atomic_uint surfelCounter;
layout(std430, binding = 10) buffer surfelSSBO { SurfelData surfels[]; };

#endif /* DEEPSCREENSPACE_SHADERS_INCLUDE_DSS_GLSL */
