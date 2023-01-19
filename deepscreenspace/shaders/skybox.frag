#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "loo/simple_material.glsl"

layout(location = 0) in vec3 vTexCoord;

out vec4 FragColor;
void main() { FragColor = texture(skyboxTex, vTexCoord); }