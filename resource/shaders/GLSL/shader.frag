#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct Material {
    vec3 albedo;
    uint64_t albedoMap;
    uint64_t metallicMap;
    uint64_t normalMap;
    float metallic;
    float smoothness;
    vec2 tilling;
    vec2 offset;
};

layout(buffer_reference, scalar) buffer Materials { Material m; };
layout(location = 0) in flat uint64_t materialAddress;
layout(location = 1) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    Materials material = Materials(materialAddress);

    //outColor = vec4(fragColor, 1.0);
    outColor = vec4(material.m.albedo, 1.0);
}