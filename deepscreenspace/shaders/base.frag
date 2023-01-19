#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "loo/simple_material.glsl"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec4 FragColor;
uniform vec3 uCameraPosition;
void main() {
    vec3 pLight = vec3(1.0);
    vec3 V = normalize(uCameraPosition - vPos);
    vec3 L = normalize(pLight - vPos);
    vec3 H = (V + L) / 2.0;

    // vec3 Ld = simpleMaterial.diffuse.rgb * max(dot(L, vNormal), 0.0);
    vec3 Ld = texture(diffuseTex, vTexCoord).rgb;
    FragColor = vec4(Ld, 1.0);
}