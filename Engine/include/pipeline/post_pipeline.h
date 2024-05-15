#pragma once
#include <pipeline/pipeline.h>

#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace core {

	class PostPipeline : public Pipeline {
	public:
		PostPipeline(VkDevice device, std::string filename, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		PostPipeline(VkDevice device, std::string filename, const std::vector<VkDescriptorSetLayout>& descSetLayouts, VkRenderPass renderPass, VkExtent2D swapChainExtent);
		~PostPipeline() = default;

	private:
		std::string filename;
		vk::RenderPass renderPass;
		vk::Extent2D swapChainExtent;

		virtual void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void createPipeline();

	};
}
