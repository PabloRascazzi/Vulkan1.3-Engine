#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform constants {
    mat4 proj;
    mat4 world;
    mat4 view;
} PushConstants;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = PushConstants.proj * PushConstants.view * PushConstants.world * vec4(inPosition, 1.0);
    fragColor = inColor;
}