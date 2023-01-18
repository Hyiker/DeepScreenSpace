layout(std140, binding = 1) uniform SimpleMaterial {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
    float ior;
    int illum;
}
simpleMaterial;

layout(binding = 2) uniform sampler2D ambientTex;
layout(binding = 3) uniform sampler2D diffuseTex;
layout(binding = 4) uniform sampler2D specularTex;
layout(binding = 5) uniform sampler2D displacementTex;
layout(binding = 6) uniform sampler2D normalTex;