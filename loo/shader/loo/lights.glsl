// lights.glsl generated by loo/xmake.lua, from lights.glsl
#ifndef LOO_TEMPLATES_LIGHTS_GLSL
#define LOO_TEMPLATES_LIGHTS_GLSL
// clang-format off

struct ShaderLight {
    // spot, point
    vec4 position;
    // spot, directional
    vec4 direction;
    // all
    vec4 color;
    float intensity;
    // point, spot
    // negative value stands for INF
    float range;
    // spot
    float spotAngle;
    int type;
};
const int LIGHT_TYPE_SPOT = 0, LIGHT_TYPE_POINT = 1, LIGHT_TYPE_DIRECTIONAL = 2;
layout(std140, binding = 1) uniform LightBlock{
    ShaderLight lights[12];
};
layout(location = 114) uniform int nLights;

#endif /* LOO_TEMPLATES_LIGHTS_GLSL */
