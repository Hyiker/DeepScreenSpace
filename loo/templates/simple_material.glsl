// clang-format off
layout(binding = %SHADER_BINDING_PORT_SKYBOX%) uniform samplerCube skyboxTex;
layout(std140, binding = %SHADER_BINDING_PORT_PBRM_PARAMS%) uniform SimpleMaterial {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
    float ior;
    int illum;
}
simpleMaterial;
layout(binding = %SHADER_BINDING_PORT_SM_AMBIENT%) uniform sampler2D ambientTex;
layout(binding = %SHADER_BINDING_PORT_SM_DIFFUSE%) uniform sampler2D diffuseTex;
layout(binding = %SHADER_BINDING_PORT_SM_SPECULAR%) uniform sampler2D specularTex;
layout(binding = %SHADER_BINDING_PORT_SM_DISPLACEMENT%) uniform sampler2D displacementTex;
layout(binding = %SHADER_BINDING_PORT_SM_NORMAL%) uniform sampler2D normalTex;
// clang-format on