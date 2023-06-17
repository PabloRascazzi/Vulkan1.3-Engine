#pragma once
#include <mesh.h>
#include <material.h>
#include <glm/matrix.hpp>
#include <camera.h>

#include <vector>
#include <unordered_set>
#include <map>
#include <stddef.h>

namespace core {

	struct Object {
		Mesh* mesh;
		std::vector<Material*> materials;
		glm::mat4 transform;
		uint32_t shaderHitGroupOffset;
	};

	struct ObjDesc {
		uint64_t vertexAddress;    // Address of the Vertex buffer
		uint64_t indexAddress;     // Address of the index buffer
		uint64_t materialAddress;  // Address of the material buffer
	};

	class Scene {
	public:
		Scene();
		~Scene();
		void setup();
		void cleanup();
		void update();

		Camera* addCamera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n = 0.01f, const float& f = 1000.f);
		Object* addObject(Mesh* mesh, std::vector<Material*> materials, glm::mat4 transform, uint32_t shader);
		Material* addMaterial(Texture* albedoMap, glm::vec3 albedo, Texture* metallicMap, float metallic, float smoothness, Texture* normalMap, glm::vec2 tilling, glm::vec2 offset);

		Camera& getMainCamera() { return *mainCamera; }
		void setMainCamera(Camera* camera);

		std::vector<Object>& getObjects() { return objects; }
		AccelerationStructure& getTLAS() { return tlas; }
		std::vector<Texture*>& getTextures() { return textures; }
		Buffer& getObjDescriptions() { return objDescBuffer; }

	private:
		Camera* mainCamera;
		std::vector<Camera*> cameras;
		std::vector<Object> objects;
		std::vector<Material*> materials;
		std::unordered_set<Mesh*> meshes;
		std::map<Texture*, uint64_t> textureIndices;

		std::vector<Texture*> textures;
		std::vector<ObjDesc> objDescriptions;

		AccelerationStructure tlas;
		Buffer objDescBuffer;

		void buildAccelerationStructure(std::vector<Object>& objects, std::unordered_set<Mesh*>& meshes);
		void createObjectDescriptions(std::vector<Object>& objects);

	};
}
