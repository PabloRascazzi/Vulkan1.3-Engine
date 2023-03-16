#pragma once
#include <mesh.h>
#include <glm/matrix.hpp>

#include <vector>
#include <unordered_set>
#include <stddef.h>

namespace core {

	struct Camera {
		glm::mat4 view;
		glm::mat4 viewInverse;
		glm::mat4 projection;
		glm::mat4 projectionInverse;
	};

	struct Object {
		Mesh* mesh;
		glm::mat4 transform;
		uint32_t shaderHitGroupOffset;
	};

	struct ObjDesc {
		uint64_t vertexAddress;    // Address of the Vertex buffer
		uint64_t indexAddress;     // Address of the index buffer
	};

	struct TopLevelAccelerationStructure {
		Buffer buffer;
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
	};

	class Scene {
	public:
		Scene();
		~Scene();
		void setup();
		void cleanup();

		Object* addObject(Mesh* mesh, glm::mat4 transform, uint32_t shader);
		TopLevelAccelerationStructure& getTLAS() { return tlas; }

	private:
		std::vector<Object> objects;
		std::unordered_set<Mesh*> meshes;
		std::vector<ObjDesc> objDescriptions;

		VkBuffer objDescBuffer;
		VmaAllocation objDescAlloc;
		TopLevelAccelerationStructure tlas;

		void buildAccelerationStructure(std::vector<Object>& objects, std::unordered_set<Mesh*> meshes);
		void createObjectDescriptions(std::vector<Object>& objects);

	};
}
