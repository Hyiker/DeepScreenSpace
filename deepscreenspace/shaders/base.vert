#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 vNormal;

layout(std140, binding = 0) uniform MVPMatrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main() {
    vPos = (model * vec4(aPos, 1.0)).xyz;
    vNormal = aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}