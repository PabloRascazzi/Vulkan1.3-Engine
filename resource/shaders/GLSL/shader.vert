#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform constants {
    mat4 proj;
    mat4 world;
    mat4 view;
    uint64_t materialAddress;
} PushConstants;

layout(location = 0) out flat uint64_t materialAddress;
layout(location = 1) out vec3 fragColor;

void main() {
    gl_Position = PushConstants.proj * PushConstants.view * PushConstants.world * vec4(inPosition, 1.0);
    materialAddress = PushConstants.materialAddress;
    fragColor = inColor;
}