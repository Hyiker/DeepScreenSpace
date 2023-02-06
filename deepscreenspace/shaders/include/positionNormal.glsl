#ifndef DEEPSCREENSPACE_SHADERS_INCLUDE_POSITION_NORMAL_GLSL
#define DEEPSCREENSPACE_SHADERS_INCLUDE_POSITION_NORMAL_GLSL

// Buffers for positions and normals of world space points associated to the
// image pixels
uniform sampler2DArray positionTexture;
uniform sampler2DArray normalTexture;

// Helper functions to sample the individual deferred buffers
vec4 sampleBuffer(const in sampler2DArray s, const in vec2 texCoord,
                  const in int layer) {
    return texelFetch(s, ivec3(texCoord, layer), 0);
}

vec4 samplePosition(const in vec2 texCoord, const in int layer) {
    return sampleBuffer(positionTexture, texCoord, layer);
}

vec3 sampleNormal(const in vec2 texCoord, const in int layer) {
    return sampleBuffer(normalTexture, texCoord, layer).rgb;
}

#endif /* DEEPSCREENSPACE_SHADERS_INCLUDE_POSITION_NORMAL_GLSL */
