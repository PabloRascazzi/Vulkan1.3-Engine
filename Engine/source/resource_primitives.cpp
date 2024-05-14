#include <resource_primitives.h>
#include <iostream>

namespace core {

	Mesh* ResourcePrimitives::createQuad(const float& edgeLength) {
        float half = (edgeLength / 2.0f);
        uint32_t vertexCount = 4;
        Vertex* vertices = new Vertex[vertexCount]{
            {{-half, -half, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom left
            {{ half, -half, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Bottom right
            {{ half,  half, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Right
            {{-half,  half, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Top Left
        };

        uint32_t indexCount = 6;
        uint32_t* indices = new uint32_t[indexCount]{
            0, 1, 2, 2, 3, 0,
        };

        uint32_t indexCountList[] = {indexCount};
        uint32_t* indicesList[] = {indices};
        return new Mesh((float*)vertices, vertexCount, 1, indicesList, indexCountList);
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
                float z = {-((static_cast<float>(edgeCount) * edgeLength) / 2.0f) + (i * edgeLength)};
                float y = 0.0f;
                float x = {-((static_cast<float>(edgeCount) * edgeLength) / 2.0f) + (j * edgeLength)};
                float r = static_cast<float>(i) / static_cast<float>(edgeCount + 1);
                float g = 0.5f;
                float b = static_cast<float>(j) / static_cast<float>(edgeCount + 1);
                float s = (1.0f / edgeCount) * static_cast<float>(j);
                float t = (1.0f / edgeCount) * static_cast<float>(i);
                vertices[idx++] = Vertex{glm::vec3(x, y, z), glm::vec3(r, g, b), glm::vec2(s, t)};
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

        uint32_t indexCountList[] = {indexCount};
        uint32_t* indicesList[] = {indices};
        return new Mesh((float*)vertices, vertexCount, 1, indicesList, indexCountList);
	}

	Mesh* ResourcePrimitives::createCube(const float& edgeLength) {
        float half = (edgeLength / 2.0f);
        uint32_t vertexCount = 24;
        Vertex* vertices = new Vertex[vertexCount]{
            // Front
            {{-half, -half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Front Bottom Left
            {{ half, -half,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Front Bottom Right
            {{ half,  half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Front Top Right
            {{-half,  half,  half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Front Top Left
            // Top
            {{-half,  half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Top Front Left
            {{ half,  half,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Top Front Right
            {{ half,  half, -half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Back Right
            {{-half,  half, -half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Top Back Left
            // Back
            {{-half,  half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Back Top Left
            {{ half,  half, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Back Top Right
            {{ half, -half, -half}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Back Bottom Right
            {{-half, -half, -half}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // Back Bottom Left
            // Bottom
            {{-half, -half, -half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom Back Left
            {{ half, -half, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Bottom Back Right
            {{ half, -half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Bottom Front Right
            {{-half, -half,  half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Bottom Front Left
            // Left
            {{-half, -half, -half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Left Bottom Back
            {{-half, -half,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Left Bottom Front
            {{-half,  half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Left Top Front
            {{-half,  half, -half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Left Top Back
            // Right 
            {{ half, -half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Right Bottom Front
            {{ half, -half, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Right Bottom Back
            {{ half,  half, -half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Right Top Back
            {{ half,  half,  half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Right Top Front
        };

        uint32_t indexCount = 36;
        uint32_t* indices = new uint32_t[indexCount]{
             0,  1,  2,  2,  3,  0, // Front
             4,  5,  6,  6,  7,  4, // Top
             8,  9, 10, 10, 11,  8, // Back
            12, 13, 14, 14, 15, 12, // Bottom
            16, 17, 18, 18, 19, 16, // Left
            20, 21, 22, 22, 23, 20, // Right
        };

        uint32_t indexCountList[] = {indexCount};
        uint32_t* indicesList[] = {indices};
        return new Mesh((float*)vertices, vertexCount, 1, indicesList, indexCountList);
	}
}