#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_debug_printf : enable

struct HitPayload {
    vec3 hitValue;
};

struct Vertex {
    vec3 position;
    vec3 color;
    vec2 uv;
};

struct Material {
    vec3 albedo;
    uint16_t albedoMapIndex;
    uint16_t metallicMapIndex;
    uint16_t normalMapIndex;
    float metallic;
    float smoothness;
    vec2 tilling;
    vec2 offset;
};

struct ObjDesc {
    uint64_t vertexAddress;    // Address of the Vertex buffer
    uint64_t indexAddress;     // Address of the index buffer
    uint64_t materialAddress;  // Address of the material buffer
};

hitAttributeEXT vec2 attribs;
layout(location = 0) rayPayloadInEXT HitPayload prd;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Reference to the array of vertices.
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Reference to the array of triangle indices.
layout(buffer_reference, scalar) buffer Materials { Material m; };
layout(binding = 0, set = 1) uniform sampler2D textures[];
layout(binding = 0, set = 2) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2, set = 2, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;

void main() {
    // Object data.
    ObjDesc objResource = objDesc.i[gl_InstanceCustomIndexEXT];
    Indices indices = Indices(objResource.indexAddress);
    Vertices vertices = Vertices(objResource.vertexAddress);
    Materials material = Materials(objResource.materialAddress);

    // Indices of the current triangle.
    ivec3 ind = indices.i[gl_PrimitiveID];

    // Vertices of the current triangle.
    Vertex v0 = vertices.v[ind.x];
    Vertex v1 = vertices.v[ind.y];
    Vertex v2 = vertices.v[ind.z];

    // Compute barycentric coordinates at hit position.
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    // Computing the coordinates of the hit position.
    const vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
    const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(position, 1.0));  // Transforming the position to world space

    // Computing the color at hit position.
    const vec3 color = v0.color * barycentrics.x + v1.color * barycentrics.y + v2.color * barycentrics.z;

    // Computing the UV coordinates at the hit position.
    const vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;

//    // Computing the normal at hit position.
//    const vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
//    const vec3 worldNrm = normalize(vec3(normal * gl_WorldToObjectEXT));  // Transforming the normal to world space

    //prd.hitValue = color;
    //prd.hitValue = material.m.albedo;
    //prd.hitValue = texture(textures[uint(material.m.albedoMap)], vec2(0.5, 0.5)).rgb;
    prd.hitValue = texture(textures[uint(material.m.albedoMapIndex)], uv).rgb;
}
