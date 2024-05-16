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
		StandardPipeline(VkDevice device, std::string shadername, VkRenderPass renderPass, VkExtent2D swapchainExtent);
		StandardPipeline(VkDevice device, std::string shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts, VkRenderPass renderPass, VkExtent2D swapchainExtent);
		~StandardPipeline() = default;

	private:
		std::string m_shadername;
		VkRenderPass m_renderPass;
		VkExtent2D m_swapchainExtent;

		virtual void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void CreatePipeline();

	};
}