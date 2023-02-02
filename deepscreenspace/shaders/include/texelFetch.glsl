#pragma once

// 2D
vec4 texelFetch2DClamp(const in sampler2D sampler, const in ivec2 texCoord,
                       const in int level) {
    return texelFetch(
        sampler,
        ivec2(max(ivec2(0),
                  min(textureSize(sampler, level) - ivec2(1), texCoord))),
        level);
}

vec4 texelFetch2DClampX(const in sampler2D sampler, const in ivec2 texCoord,
                        const in int level) {
    return texelFetch(
        sampler,
        ivec2(max(0, min(textureSize(sampler, level).x - 1, texCoord.x)),
              texCoord.y),
        level);
}

vec4 texelFetch2DClampY(const in sampler2D sampler, const in ivec2 texCoord,
                        const in int level) {
    return texelFetch(
        sampler,
        ivec2(texCoord.x,
              max(0, min(textureSize(sampler, level).y - 1, texCoord.y))),
        level);
}

vec4 texelFetch2DRepeat(const in sampler2D sampler, const in ivec2 texCoord,
                        const int level) {
    return texelFetch(sampler, texCoord % textureSize(sampler, level), level);
}

vec4 texelFetch2DRepeat(const in sampler2D sampler, const in ivec2 texCoord) {
    return texelFetch2DRepeat(sampler, texCoord, 0);
}

vec4 texelFetch2DMirror(const in sampler2D sampler, const in ivec2 texCoord,
                        const int level) {
    ivec2 lookupTexCoord = texCoord;
    ivec2 res = textureSize(sampler, 0).xy;
    lookupTexCoord.x = lookupTexCoord.x < 0 ? res.x - (lookupTexCoord.x + res.x)
                                            : lookupTexCoord.x;
    lookupTexCoord.x = lookupTexCoord.x >= res.x
                           ? res.x - 1 - (lookupTexCoord.x - res.x)
                           : lookupTexCoord.x;
    lookupTexCoord.y = lookupTexCoord.y < 0 ? res.y - (lookupTexCoord.y + res.y)
                                            : lookupTexCoord.y;
    lookupTexCoord.y = lookupTexCoord.y >= res.y
                           ? res.y - 1 - (lookupTexCoord.y - res.y)
                           : lookupTexCoord.y;
    return texelFetch(sampler, lookupTexCoord, level);
}

vec4 texelFetch2DMirror(const in sampler2D sampler, const in ivec2 texCoord) {
    return texelFetch2DMirror(sampler, texCoord, 0);
}

vec4 texelFetch2DArrayMirror(const in sampler2DArray sampler,
                             const in ivec3 texCoord) {
    ivec2 lookupTexCoord = texCoord.xy;
    ivec2 res = textureSize(sampler, 0).xy;
    lookupTexCoord.x = lookupTexCoord.x < 0 ? res.x - (lookupTexCoord.x + res.x)
                                            : lookupTexCoord.x;
    lookupTexCoord.x = lookupTexCoord.x >= res.x
                           ? res.x - 1 - (lookupTexCoord.x - res.x)
                           : lookupTexCoord.x;
    lookupTexCoord.y = lookupTexCoord.y < 0 ? res.y - (lookupTexCoord.y + res.y)
                                            : lookupTexCoord.y;
    lookupTexCoord.y = lookupTexCoord.y >= res.y
                           ? res.y - 1 - (lookupTexCoord.y - res.y)
                           : lookupTexCoord.y;
    return texelFetch(sampler, ivec3(lookupTexCoord, texCoord.z), 0);
}

// if the texel is outside the image return a color
vec4 texelFetch2DBorderColor(const in sampler2D sampler,
                             const in ivec2 texCoord, const in int level,
                             const vec4 color) {
    ivec2 size = textureSize(sampler, level).xy;
    return texCoord.x < size.x && texCoord.y < size.y && texCoord.x >= 0 &&
                   texCoord.y >= 0
               ? texelFetch(sampler, texCoord, level)
               : color;
}

vec4 texelFetch2DBorderZero(const in sampler2D sampler, const in ivec2 texCoord,
                            const in int level) {
    return texelFetch2DBorderColor(sampler, texCoord, level, vec4(0));
}

// 2D_ARRAY

vec4 texelFetch2DArrayClamp(const in sampler2DArray sampler,
                            const in ivec2 texCoord, const in int layer,
                            const in int level) {
    return texelFetch(
        sampler,
        ivec3(max(ivec2(0),
                  min(textureSize(sampler, level).xy - ivec2(1), texCoord)),
              layer),
        level);
}

// 3D

vec4 texelFetch3DClamp(const in sampler3D sampler, const in ivec3 texCoord,
                       const in int level) {
    return texelFetch(sampler,
                      ivec3(clamp(texCoord, ivec3(0),
                                  textureSize(sampler, level) - ivec3(1))),
                      level);
}

vec4 texelFetch3DRepeat(const sampler3D texture3D, const ivec3 texCoord,
                        const int level) {  // default argument was 0
    return texelFetch(
        texture3D, (texCoord + ivec3(1000000)) % textureSize(texture3D, level),
        level);
}

vec4 texelFetch3DRepeat(const sampler3D texture3D, const ivec3 texCoord) {
    return texelFetch3DRepeat(texture3D, texCoord, 0);
}

vec4 texelFetch3DBorderColor(const in sampler3D sampler,
                             const in ivec3 texCoord, const in int level,
                             const vec4 color) {
    ivec3 size = textureSize(sampler, level).xyz;
    return clamp(texCoord, ivec3(0), textureSize(sampler, level) - ivec3(1)) ==
                   texCoord
               ? texelFetch(sampler, texCoord, level)
               : color;
}

vec4 texelFetch3DBorderZero(const in sampler3D sampler, const in ivec3 texCoord,
                            const in int level) {
    return texelFetch3DBorderColor(sampler, texCoord, level, vec4(0));
}

// 0D - a.k.a fragFetch: No coord parameters.

vec4 fragFetch(const sampler2D s) {
    return texelFetch(s, ivec2(gl_FragCoord.xy), 0);
}

vec4 fragFetchOffset(const sampler2D s, const ivec2 offset) {
    return texelFetch(s, ivec2(gl_FragCoord.xy) + offset, 0);
}

vec4 fragFetchOffsetMirror(const sampler2D s, const ivec2 offset) {
    return texelFetch2DMirror(s, ivec2(gl_FragCoord.xy) + offset);
}

vec4 fragFetchRepeat(const in sampler2D sampler) {
    return texelFetch2DRepeat(sampler, ivec2(gl_FragCoord.xy));
}

vec4 fragFetchArray(const sampler2DArray s, const int layerIndex) {
    return texelFetch(s, ivec3(gl_FragCoord.xy, layerIndex), 0);
}

ivec4 fragFetch(const isampler2D s) {
    return texelFetch(s, ivec2(gl_FragCoord.xy), 0);
}

ivec4 fragFetchOffset(const isampler2D s, const ivec2 offset) {
    return texelFetch(s, ivec2(gl_FragCoord.xy) + offset, 0);
}

uvec4 fragFetch(const usampler2D s) {
    return texelFetch(s, ivec2(gl_FragCoord.xy), 0);
}

uvec4 fragFetchOffset(const usampler2D s, const ivec2 offset) {
    return texelFetch(s, ivec2(gl_FragCoord.xy) + offset, 0);
}

// Detects if a sampler can be read from the frag coord + some offset
bool fragOffsetInSampler(const sampler2D s, const ivec2 offset) {
    return all(lessThan(ivec2(gl_FragCoord.xy) + offset, textureSize(s, 0))) &&
           all(greaterThan(ivec2(gl_FragCoord.xy) + offset, ivec2(-1)));
}
