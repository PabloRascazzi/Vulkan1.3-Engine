#pragma once
#include <pipeline/pipeline.h>

#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace core {

	struct StandardPushConstant {
		glm::mat4 proj;
		glm::mat4 world;
		glm::mat4 view;

		static uint32_t getSize() {
			return sizeof(StandardPushConstant);
		}
	};

	class StandardPipeline : Pipeline {
	public:
		StandardPipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		StandardPipeline(VkDevice device, std::vector<DescriptorSet*> descriptorSets, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		~StandardPipeline();
		virtual void cleanup();

	private:
		vk::RenderPass renderPass;
		vk::Extent2D swapChainExtent;

		virtual void createPipelineLayout();
		virtual void createPipeline();

	};
}