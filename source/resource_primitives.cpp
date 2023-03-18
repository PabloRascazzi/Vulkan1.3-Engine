#include <resource_primitives.h>
#include <iostream>

namespace core {

	Mesh* ResourcePrimitives::createQuad(const float& edgeLength) {
        uint32_t vertexCount = 4;
        Vertex* vertices = new Vertex[vertexCount]{
            {{-(edgeLength/2.0f), -(edgeLength/2.0f), 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom left
            {{ (edgeLength/2.0f), -(edgeLength/2.0f), 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom right
            {{ (edgeLength/2.0f),  (edgeLength/2.0f), 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top Right
            {{-(edgeLength/2.0f),  (edgeLength/2.0f), 0.0f}, {1.0f, 1.0f, 1.0f}}, // Top Left
        };

        uint32_t indexCount = 6;
        uint32_t* indices = new uint32_t[indexCount]{
            0, 1, 2, 2, 3, 0,
        };

        return new Mesh((float*)vertices, vertexCount, indices, indexCount);
	}

	Mesh* ResourcePrimitives::createPlane(const uint32_t& edgeCount, const float& edgeLength) {
        // Check that edgeCount is even and greater than 0.
        if (edgeCount%2 != 0 || edgeCount == 0) return nullptr;

        uint32_t vertexCount = (edgeCount + 1) * (edgeCount+1);
        Vertex* vertices = new Vertex[vertexCount];
        uint32_t idx = 0;
        for (uint32_t i = 0; i < edgeCount + 1; i++) {
            for (uint32_t j = 0; j < edgeCount + 1; j++) {
                // To flip up direction of the plane, flip 'x' and 'z'.
                float z = {i - ((static_cast<float>(edgeCount) / 2.0f) * edgeLength)};
                float y = 0.0f;
                float x = {j - ((static_cast<float>(edgeCount) / 2.0f) * edgeLength)};
                float r = static_cast<float>(i) / static_cast<float>(edgeCount + 1);
                float g = 0.5f;
                float b = static_cast<float>(j) / static_cast<float>(edgeCount + 1);
                vertices[idx++] = Vertex{glm::vec3(x, y, z), glm::vec3(r, g, b)};
            }
        }

        uint32_t indexCount = edgeCount*edgeCount*6;
        uint32_t* indices = new uint32_t[indexCount];
        uint32_t idx2 = 0;
        for (uint32_t i = 0; i < edgeCount; i++) {
            for (uint32_t j = 0; j < edgeCount; j++) {
                uint32_t a = (edgeCount + 1) + j;
                uint32_t b = (edgeCount + 1) * i;
                indices[idx2++] = b+a;
                indices[idx2++] = b+a+1;
                indices[idx2++] = b+j+1;
                indices[idx2++] = b+j+1;
                indices[idx2++] = b+j;
                indices[idx2++] = b+a;
            }
        }

        return new Mesh((float*)vertices, vertexCount, indices, indexCount);
	}

	Mesh* ResourcePrimitives::createCube(const float& edgeLength) {
        return nullptr;
	}
}