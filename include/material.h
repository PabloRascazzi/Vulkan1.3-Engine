#pragma once
#include <resource_allocator.h>
#include <texture.h>
#include <glm/glm.hpp>

namespace core {

	class Material {
	public:
		Material(Texture* albedoMap, glm::vec3 albedo, Texture* metallicMap, float metallic, float smoothness, Texture* normalMap, glm::vec2 tilling, glm::vec2 offset);
		~Material();

		void cleanup();
		void setup(uint64_t albedoMapIndex, uint64_t metallicMapIndex, uint64_t normalMapIndex);

		Buffer& getBuffer() { return materialBuffer; }

	private:
		Texture* albedoMap;   // Texture map which represents the color of the surface without lighting or shadowing.
		glm::vec3 albedo;     // Vector 3D which reprsents the color of the surface without lighting or shadowing.
		Texture* metallicMap; // Texture map which represents how the surface react to light.
		float metallic;       // Floating value of range [0.0, 1.0] which represents how the surface react to light (ONLY USED IF "metallicMap" IS NULL).
		float smoothness;     // Floating value of range [0.0, 1.0] which controls how the light is spread accross the surface.
		Texture* normalMap;   // Texture map which represent the normal vectors of the surface.
		glm::vec2 tilling;    // Vector 2D which scales all the mapped textures.
		glm::vec2 offset;     // Vector 2D which offsets all the mapped textures.

		uint64_t albedoMapIndex;
		uint64_t metallicMapIndex;
		uint64_t normalMapIndex;
		Buffer materialBuffer; // On device buffer containing the material's data.

		void createMaterialBuffer(Buffer& buffer);

	};
}
