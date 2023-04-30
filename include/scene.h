#pragma once
#include <mesh.h>
#include <glm/matrix.hpp>
#include <camera.h>

#include <vector>
#include <unordered_set>
#include <stddef.h>

namespace core {

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
		void update();

		Camera* addCamera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n = 0.01f, const float& f = 1000.f);
		Object* addObject(Mesh* mesh, glm::mat4 transform, uint32_t shader);

		Camera& getMainCamera() { return *mainCamera; }
		void setMainCamera(Camera* camera);

		std::vector<Object>& getObjects() { return objects; }
		TopLevelAccelerationStructure& getTLAS() { return tlas; }
		Buffer& getObjDescriptions() { return objDescBuffer; }

	private:
		Camera* mainCamera;
		std::vector<Camera*> cameras;
		std::vector<Object> objects;
		std::unordered_set<Mesh*> meshes;
		std::vector<ObjDesc> objDescriptions;

		TopLevelAccelerationStructure tlas;
		Buffer objDescBuffer;

		void buildAccelerationStructure(std::vector<Object>& objects, std::unordered_set<Mesh*> meshes);
		void createObjectDescriptions(std::vector<Object>& objects);

	};
}
