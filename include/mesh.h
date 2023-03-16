#pragma once
#include <glm/glm.hpp>
#include <Vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

#include <resource_allocator.h>

namespace core {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = 0;
			bindingDesc.stride = sizeof(Vertex);
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDesc;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			return attributeDescriptions;
		}
	};

	struct BottomLevelAccelerationStructure {
		Buffer buffer;
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
		VkDeviceAddress getDeviceAddress();
	};

	class Mesh {
	public:
		Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);
		~Mesh();

		void cleanup();

		Buffer& getVertexBuffer() { return vertexBuffer; }
		Buffer& getIndexBuffer() { return indexBuffer; }
		uint32_t getVertexCount() { return vertexCount; }
		uint32_t getIndexCount() { return indexCount; }
		BottomLevelAccelerationStructure& getBLAS() { return blas; }

	private:
		uint32_t vertexCount;
		uint32_t indexCount;

		Buffer vertexBuffer;
		Buffer indexBuffer;
		BottomLevelAccelerationStructure blas;

		void createVertexBuffer(Vertex* vertices);
		void createIndexBuffer(uint32_t* indices);

	};
}