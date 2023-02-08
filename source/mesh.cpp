#include <mesh.h>
#include <engine_context.h>

namespace core {

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
		EngineContext::destroyBuffer(vertexBuffer, vertexAlloc);
		EngineContext::destroyBuffer(indexBuffer, indexAlloc);
	}

	void Mesh::createVertexBuffer(Vertex* vertices) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VkBuffer stageVertexBuffer;
		VmaAllocation stageVertexAlloc;
		VkBufferUsageFlags stageBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		EngineContext::createBuffer(bufferSize, stageBufferUsage, stageVertexBuffer, stageVertexAlloc);
		EngineContext::mapBufferData(stageVertexAlloc, (size_t)bufferSize, vertices);

		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		EngineContext::createBuffer(bufferSize, bufferUsage, vertexBuffer, vertexAlloc);
		EngineContext::copyBufferData(stageVertexBuffer, vertexBuffer, bufferSize);

		EngineContext::destroyBuffer(stageVertexBuffer, stageVertexAlloc);
	}

	void Mesh::createIndexBuffer(uint32_t* indices) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		
		VkBuffer stageIndexBuffer;
		VmaAllocation stageIndexAlloc;
		VkBufferUsageFlags stageBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		EngineContext::createBuffer(bufferSize, stageBufferUsage, stageIndexBuffer, stageIndexAlloc);
		EngineContext::mapBufferData(stageIndexAlloc, (size_t)bufferSize, indices);

		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		EngineContext::createBuffer(bufferSize, bufferUsage, indexBuffer, indexAlloc);
		EngineContext::copyBufferData(stageIndexBuffer, indexBuffer, bufferSize);

		EngineContext::destroyBuffer(stageIndexBuffer, stageIndexAlloc);
	}
}