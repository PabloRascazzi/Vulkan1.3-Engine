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
		VkDeviceAddress materialAddress;

		static uint32_t getSize() {
			return sizeof(StandardPushConstant);
		}
	};

	class StandardPipeline : public Pipeline {
	public:
		StandardPipeline(VkDevice device, std::string filename, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		StandardPipeline(VkDevice device, std::string filename, std::vector<VkDescriptorSetLayout> descSetLayouts, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		~StandardPipeline();
		virtual void cleanup();

	private:
		std::string filename;
		vk::RenderPass renderPass;
		vk::Extent2D swapChainExtent;

		virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& layouts);
		virtual void createPipeline();

	};
}