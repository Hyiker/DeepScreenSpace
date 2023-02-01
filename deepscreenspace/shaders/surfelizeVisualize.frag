#version 460 core
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

out vec4 FragColor;
void main() { FragColor = vec4(vNormal, 1); }