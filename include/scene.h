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

	class Scene {
	public:
		Scene();
		~Scene();
		void setup();
		void cleanup();

		Object* addObject(Mesh* mesh, glm::mat4 transform, uint32_t shader);

	private:
		std::vector<Object> objects;

		void buildAS(std::vector<Object>& objects);

	};
}
