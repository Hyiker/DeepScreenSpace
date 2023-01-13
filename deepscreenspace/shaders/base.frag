#version 460 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

out vec4 FragColor;
layout(location = 3) uniform vec3 uCameraPosition;
layout(location = 4) uniform struct {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} material;
void main() {
    vec3 pLight = vec3(1.0);
    vec3 V = normalize(uCameraPosition - vPos);
    vec3 L = normalize(pLight - vPos);
    vec3 H = (V + L) / 2.0;
    FragColor = vec4(abs(dot(L, normalize(vNormal))) * material.diffuse, 1.0f);
}