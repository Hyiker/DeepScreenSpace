// simple_material.glsl generated by loo/xmake.lua, from simple_material.glsl
// clang-format off
layout(binding = 0) uniform samplerCube skyboxTex;
layout(std140, binding = 2) uniform SimpleMaterial {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
    float ior;
    int illum;
}
simpleMaterial;
layout(binding = 3) uniform sampler2D ambientTex;
layout(binding = 4) uniform sampler2D diffuseTex;
layout(binding = 5) uniform sampler2D specularTex;
layout(binding = 6) uniform sampler2D displacementTex;
layout(binding = 7) uniform sampler2D normalTex;
layout(binding = 8) uniform sampler2D opacityTex;
layout(binding = 9) uniform sampler2D heightTex;
// clang-format on
