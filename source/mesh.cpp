#include <mesh.h>
#include <engine_context.h>

namespace core {

	Mesh::Submesh::Submesh(uint32_t* indices, uint32_t indexCount) {
		this->indexCount = indexCount;
		createIndexBuffer(indices, indexCount, this->indexBuffer);
	}

	Mesh::Submesh::~Submesh() {
		cleanup();
	}

	void Mesh::Submesh::cleanup() {
		ResourceAllocator::destroyBuffer(indexBuffer);
		ResourceAllocator::destroyAccelerationStructure(blas);
	}

	Mesh::Mesh(float* vertices, uint32_t vertexCount, uint32_t submeshCount, uint32_t** indicesList, uint32_t* indexCountList) {
		// Create vertices
		this->vertexCount = vertexCount;
		createVertexBuffer((Vertex*)vertices, vertexCount, this->vertexBuffer);

		// Create Submeshes
		this->submeshCount = submeshCount;
		this->submeshes = new Submesh*[submeshCount];
		for (uint32_t i = 0; i < submeshCount; i++) {
			// Create submesh object using indices array and index count
			this->submeshes[i] = new Submesh(indicesList[i], indexCountList[i]);
			// Delete indices array after copied into submesh buffer
			delete indicesList[i];
		}
	}

	Mesh::~Mesh() {
		cleanup();
	}

	void Mesh::cleanup() {
		ResourceAllocator::destroyBuffer(vertexBuffer);
		for (uint32_t i = 0; i < submeshCount; i++) 
			delete submeshes[i];
		delete[] submeshes;
	}

	void Mesh::createVertexBuffer(Vertex* vertices, uint32_t vertexCount, Buffer& vertexBuffer) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		ResourceAllocator::createAndStageBuffer(bufferSize, vertices, vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	}

	void Mesh::createIndexBuffer(uint32_t* indices, uint32_t indexCount, Buffer& indexBuffer) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		ResourceAllocator::createAndStageBuffer(bufferSize, indices, indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	}
}