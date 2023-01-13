#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 vNormal;

layout(location = 0) uniform mat4 uView;
layout(location = 1) uniform mat4 uProj;
layout(location = 2) uniform mat4 uModel;

void main() {
    vPos = (uModel * vec4(aPos, 1.0)).xyz;
    vNormal = aNormal;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}