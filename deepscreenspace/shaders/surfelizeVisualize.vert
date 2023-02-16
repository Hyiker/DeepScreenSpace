#version 460 core
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aRadius;

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) flat out float vRadius;

layout(std140, binding = 0) uniform MVPMatrices {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normalMatrix;
};

void main() {
    vPos = aPos;
    vNormal = normalize(aNormal);
    vRadius = aRadius;
    gl_Position = projection * view * vec4(vPos, 1);
    gl_PointSize = 2.0;
}