#pragma once
#include <mesh.h>

namespace core {

	class ResourcePrimitives {
	public:
		static Mesh* createQuad(const float& edgeLength);
		static Mesh* createPlane(const uint32_t& edgeCount, const float& edgeLength);
		static Mesh* createCube(const float& edgeLength);

	};
}
