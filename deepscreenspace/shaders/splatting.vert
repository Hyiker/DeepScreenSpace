#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "include/dss.glsl"
#include "include/sss.glsl"
#include "include/surfel.glsl"

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aRadius;

layout(location = 0) flat out Surfel vSurfel;

void main() { vSurfel = initSurfel(aPos.xyz, aNormal, aRadius); }