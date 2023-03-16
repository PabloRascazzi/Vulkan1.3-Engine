#include <mesh.h>
#include <engine_context.h>

namespace core {

	VkDeviceAddress BottomLevelAccelerationStructure::getDeviceAddress() {
		return buffer.getDeviceAddress();
	}

	Mesh::Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount) {
		this->vertexCount = vertexCount;
		this->indexCount = indexCount;
		createVertexBuffer((Vertex*)vertices);
		createIndexBuffer(indices);
	}

	Mesh::~Mesh() {
		cleanup();
	}

	void Mesh::cleanup() {
		ResourceAllocator::destroyBuffer(vertexBuffer);
		ResourceAllocator::destroyBuffer(indexBuffer);
		ResourceAllocator::destroyBuffer(blas.buffer);
		EngineContext::getDevice().destroyAccelerationStructureKHR(blas.handle);
	}

	void Mesh::createVertexBuffer(Vertex* vertices) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		ResourceAllocator::createAndStageBuffer(bufferSize, vertices, vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	}

	void Mesh::createIndexBuffer(uint32_t* indices) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		ResourceAllocator::createAndStageBuffer(bufferSize, indices, indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	}
}