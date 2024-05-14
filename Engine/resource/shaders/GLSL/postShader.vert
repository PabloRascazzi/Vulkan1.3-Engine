#version 450
layout(location = 0) out vec2 outUv;

layout(binding = 0, set = 0) uniform sampler2D texSampler;

vec3 positions[4] = vec3[](
    vec3(-1.0f, -1.0f, 0.0f), // Top-Left vertex
    vec3(-1.0f,  1.0f, 0.0f), // Bottom-Left vertex
    vec3( 1.0f, -1.0f, 0.0f), // Top-Right vertex
    vec3( 1.0f,  1.0f, 0.0f)  // Bottom-Right vertex
);

vec2 uvs[4] = vec2[](
    vec2(0.0f, 0.0f), // Top-Left uv
    vec2(0.0f, 1.0f), // Bottom-Left uv
    vec2(1.0f, 0.0f), // Top-Right uv
    vec2(1.0f, 1.0f)  // Bottom-Right uv
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
    outUv = uvs[gl_VertexIndex];
}