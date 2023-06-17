#pragma once
#include <glm/glm.hpp>
#include <Vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

#include <resource_allocator.h>

namespace core {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = 0;
			bindingDesc.stride = sizeof(Vertex);
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDesc;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, uv);
			return attributeDescriptions;
		}
	};

	class Mesh {
	public:
		class Submesh {
		public: 
			Submesh(uint32_t* indices, uint32_t indexCount);
			~Submesh();

			void cleanup();

			uint32_t getIndexCount() { return indexCount; }
			Buffer& getIndexBuffer() { return indexBuffer; }
			AccelerationStructure& getBLAS() { return blas; }

		private:
			uint32_t indexCount;
			Buffer indexBuffer;
			AccelerationStructure blas;

		};

	public:
		Mesh(float* vertices, uint32_t vertexCount, uint32_t submeshCount, uint32_t** indicesList, uint32_t* indexCountList);
		~Mesh();

		void cleanup();

		Buffer& getVertexBuffer() { return vertexBuffer; }
		uint32_t getVertexCount() { return vertexCount; }
		Submesh& getSubmesh(uint32_t index) { return *submeshes[index]; }
		uint32_t& getSubmeshCount() { return submeshCount; }

	private:
		uint32_t vertexCount;
		Buffer vertexBuffer;
		uint32_t submeshCount;
		Submesh** submeshes;

		static void createVertexBuffer(Vertex* vertices, uint32_t vertexCount, Buffer& vertexBuffer);
		static void createIndexBuffer(uint32_t* indices, uint32_t indexCount, Buffer& indexBuffer);

	};
}