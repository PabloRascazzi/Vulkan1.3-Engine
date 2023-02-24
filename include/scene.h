#pragma once
#include <mesh.h>
#include <glm/matrix.hpp>

#include <vector>
#include <stddef.h>

namespace core {

	struct Object {
		Mesh* mesh;
		glm::mat4 transform;
		uint32_t shaderHitGroupOffset;
	};

	struct TopLevelAccelerationStructure {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation alloc = VK_NULL_HANDLE;
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
	};

	class Scene {
	public:
		Scene();
		~Scene();
		void setup();
		void cleanup();

		Object* addObject(Mesh* mesh, glm::mat4 transform, uint32_t shader);

	private:
		std::vector<Object> objects;
		TopLevelAccelerationStructure tlas;

		void buildAccelerationStructure(std::vector<Object>& objects);

	};
}
