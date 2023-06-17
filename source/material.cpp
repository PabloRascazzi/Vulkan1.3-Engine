#include <material.h>

namespace core {

	// Material data structure (Must match the shader's material structure).
	struct MaterialData {
		alignas(16) glm::vec3 albedo;     // 16 bytes
		alignas(8)  uint64_t albedoMap;   //  8 bytes
		alignas(8)  uint64_t metallicMap; //  8 bytes
		alignas(8)  uint64_t normalMap;   //  8 bytes
		alignas(4)  float metallic;       //  4 bytes
		alignas(4)  float smoothness;     //  4 bytes
		alignas(8)  glm::vec2 tilling;    //  8 bytes
		alignas(8)  glm::vec2 offset;     //  8 bytes
	};

	Material::Material(Texture* albedoMap, glm::vec3 albedo, Texture* metallicMap, float metallic, float smoothness, Texture* normalMap, glm::vec2 tilling, glm::vec2 offset) 
		: albedoMap(albedoMap), albedo(albedo), metallicMap(metallicMap), metallic(metallic), smoothness(smoothness), normalMap(normalMap), tilling(tilling), offset(offset) {
		this->albedoMapIndex = 0xFFFFFFFFFFFFFFFF;
		this->metallicMapIndex = 0xFFFFFFFFFFFFFFFF;
		this->normalMapIndex = 0xFFFFFFFFFFFFFFFF;
	}

	Material::~Material() { 
		cleanup(); 
	}

	void Material::cleanup() {
		ResourceAllocator::destroyBuffer(materialBuffer);
	}

	void Material::setup(uint64_t albedoMapIndex, uint64_t metallicMapIndex, uint64_t normalMapIndex) {
		this->albedoMapIndex = albedoMapIndex;
		this->metallicMapIndex = metallicMapIndex;
		this->normalMapIndex = normalMapIndex;
		createMaterialBuffer(materialBuffer);
	}

	void Material::createMaterialBuffer(Buffer& buffer) {
		// Create material data.
		MaterialData data{albedo, albedoMapIndex, metallicMapIndex, normalMapIndex, metallic, smoothness, tilling, offset};

		// Create and stage buffer.
		ResourceAllocator::createAndStageBuffer(sizeof(MaterialData), &data, buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	}

}