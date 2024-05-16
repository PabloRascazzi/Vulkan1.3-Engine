#pragma once
#include <pipeline/pipeline.h>

#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace core {

	class PostPipeline : public Pipeline {
	public:
		PostPipeline(VkDevice device, std::string shadername, VkRenderPass renderPass, VkExtent2D swapchainExtent);
		PostPipeline(VkDevice device, std::string shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts, VkRenderPass renderPass, VkExtent2D swapchainExtent);
		~PostPipeline() = default;

	private:
		std::string m_shadername;
		VkRenderPass m_renderPass;
		VkExtent2D m_swapchainExtent;

		virtual void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void CreatePipeline();

	};
}
