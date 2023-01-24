#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "loo/lights.glsl"
#include "loo/simple_material.glsl"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec4 FragColor;
uniform vec3 uCameraPosition;

void main() {
    vec3 V = normalize(uCameraPosition - vPos);
    vec3 color = vec3(0);
    for (int i = 0; i < nLights; i++) {
        ShaderLight light = lights[i];
        float distance = 1.0;
        vec3 L, H;
        switch (light.type) {
            case LIGHT_TYPE_DIRECTIONAL:
                L = normalize(-lights[0].direction.xyz);
                H = (V + L) / 2.0;

                break;
            default:
                continue;
        }
        float attenuation = light.intensity / (distance * distance);
        vec3 Ld = texture(diffuseTex, vTexCoord).rgb *
                  simpleMaterial.diffuse.rgb * attenuation *
                  max(dot(L, vNormal), 0.0);
        vec3 Ls = texture(specularTex, vTexCoord).rgb *
                  simpleMaterial.specular.rgb * attenuation *
                  max(0.0, dot(H, vNormal));
        color += Ld + Ls;
    }
    vec3 Ld = texture(diffuseTex, vTexCoord).rgb;
    FragColor = vec4(color, 1.0);
}