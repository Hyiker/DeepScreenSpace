#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "loo/lights.glsl"
#include "loo/simple_material.glsl"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
// tangent space -> world space
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

out vec4 FragColor;
uniform vec3 uCameraPosition;
layout(location = 18) uniform bool enableNormal;
layout(location = 19) uniform bool enableParallax;
layout(location = 20) uniform bool enableLodVisualize;
layout(location = 21) uniform int meshLod;
const float ambientIntensity = 0.2f;

vec2 parallaxMapping(vec2 texCoord, vec3 viewTS) {
    float height = texture(heightTex, texCoord).r;
    vec2 p = viewTS.xy / viewTS.z * (height * 0.2);
    return texCoord + p;
}
void main() {
    if (enableLodVisualize) {
        switch (meshLod) {
            case 0:
                FragColor.rgb = vec3(0.50, 0, 0.15);
                break;
            case 1:
                FragColor.rgb = vec3(1, 0.31, 0.16);
                break;
            case 2:
                FragColor.rgb = vec3(1, 1, 0.8);
                break;
        }
        return;
    }

    vec3 V = normalize(uCameraPosition - vPos);
    vec3 color = vec3(0);
    mat3 TBN =
        mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    // TBN = transpose(TBN);
    // compute parallax mapped texture coordinates
    // vec3 cameraTS = iTBN * uCameraPosition;
    // vec3 fragTS = iTBN * vPos;
    // vec3 viewTS = normalize(cameraTS - fragTS);
    vec2 texCoord = vTexCoord;

    // shading normal
    vec3 sNormal = texture(normalTex, texCoord).rgb;
    sNormal = length(sNormal) == 0.0 ? vNormal : (TBN * (sNormal * 2.0 - 1.0));
    sNormal = normalize(enableNormal ? sNormal : vNormal);
    float opacity = texture(diffuseTex, texCoord).a;
    if (opacity == 0.0) {
        // just discard all transparent fragments
        discard;
    }
    for (int i = 0; i < nLights; i++) {
        ShaderLight light = lights[i];
        float distance = 1.0;
        vec3 L, H;
        switch (light.type) {
            case LIGHT_TYPE_DIRECTIONAL:
                L = normalize(-lights[0].direction.xyz);
                H = normalize(V + L);
                break;
            default:
                continue;
        }
        float attenuation = light.intensity / (distance * distance);
        vec3 Ld = texture(diffuseTex, texCoord).rgb *
                  simpleMaterial.diffuse.rgb * attenuation *
                  max(dot(L, sNormal), 0.0);
        vec3 Ls = texture(specularTex, texCoord).rgb *
                  simpleMaterial.specular.rgb * attenuation *
                  pow(max(0.0, dot(H, sNormal)), simpleMaterial.shininess);
        color += Ld + Ls;
    }
    vec3 La = texture(ambientTex, texCoord).rgb * simpleMaterial.ambient.rgb *
              ambientIntensity;
    color += La;
    FragColor = vec4(color, 1.0);
}