#version 460 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

out vec4 FragColor;
layout(location = 0) uniform vec3 uCameraPosition;
layout(std140, binding = 1) uniform SimpleMaterial {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
    float ior;
    int illum;
}
simpleMaterial;
void main() {
    vec3 pLight = vec3(1.0);
    vec3 V = normalize(uCameraPosition - vPos);
    vec3 L = normalize(pLight - vPos);
    vec3 H = (V + L) / 2.0;
    FragColor = simpleMaterial.diffuse;
}