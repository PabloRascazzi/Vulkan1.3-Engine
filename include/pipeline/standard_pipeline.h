#pragma once
#include <pipeline/pipeline.h>

#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace core {

	struct StandardPushConstants {
		glm::mat4 proj;
		glm::mat4 world;
		glm::mat4 view;

		static uint32_t getSize() {
			return sizeof(StandardPushConstants);
		}
	};

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

	class StandardPipeline : Pipeline {
	public:
		StandardPipeline() {};
		~StandardPipeline() { cleanup(); };
		virtual void setup(VkDevice device, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		virtual void cleanup();

	private:
		virtual void createPipelineLayout();
		virtual void createPipeline();

	};
}