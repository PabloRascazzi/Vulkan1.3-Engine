#include <resource_primitives.h>
#include <iostream>

namespace core {

	Mesh* ResourcePrimitives::createQuad(const float& edgeLength) {
        float half = (edgeLength / 2.0f);
        uint32_t vertexCount = 4;
        Vertex* vertices = new Vertex[vertexCount]{
            {{-half, -half, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom left
            {{ half, -half, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom right
            {{ half,  half, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top Right
            {{-half,  half, 0.0f}, {1.0f, 1.0f, 1.0f}}, // Top Left
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
        float half = (edgeLength / 2.0f);
        uint32_t vertexCount = 8;
        Vertex* vertices = new Vertex[vertexCount]{
            {{-half, -half,  half}, {1.0f, 0.0f, 0.0f}}, // Front Bottom left
            {{ half, -half,  half}, {0.0f, 1.0f, 0.0f}}, // Front Bottom right
            {{ half,  half,  half}, {0.0f, 0.0f, 1.0f}}, // Front Top Right
            {{-half,  half,  half}, {1.0f, 1.0f, 1.0f}}, // Front Top Left
            {{-half, -half, -half}, {1.0f, 0.0f, 0.0f}}, // Back Bottom left
            {{ half, -half, -half}, {0.0f, 1.0f, 0.0f}}, // Back Bottom right
            {{ half,  half, -half}, {0.0f, 0.0f, 1.0f}}, // Back Top Right
            {{-half,  half, -half}, {1.0f, 1.0f, 1.0f}}, // Back Top Left
        };

        uint32_t indexCount = 36;
        uint32_t* indices = new uint32_t[indexCount]{
            0, 1, 2, 2, 3, 0, // Front
            6, 5, 4, 4, 7, 6, // Back
            7, 4, 0, 0, 3, 7, // Left
            2, 1, 5, 5, 6, 2, // Right
            7, 3, 2, 2, 6, 7, // Top
            0, 4, 5, 5, 1, 0, // Bottom
        };

        return new Mesh((float*)vertices, vertexCount, indices, indexCount);
	}
}