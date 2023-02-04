#pragma once

#include <vulkan/vulkan.hpp>

namespace core {

	enum class PipelineType {
		PIPELINE_TYPE_NONE, 
		PIPELINE_TYPE_RASTERIZATION, 
		PIPELINE_TYPE_RAY_TRACING,
	};

	class Pipeline {
	public:
		Pipeline() {}
		virtual void setup(VkDevice device, VkRenderPass renderPass, VkExtent2D swapChainExtent) = 0;
		virtual void cleanup() = 0;

		vk::PipelineLayout getLayout() { return layout; }
		vk::Pipeline getHandle() { return pipeline; }
		PipelineType getType() { return type; }

	protected:
		vk::Device device;
		vk::RenderPass renderPass;
		vk::Extent2D swapChainExtent;

		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
		PipelineType type;

		virtual void createPipelineLayout() = 0;
		virtual void createPipeline() = 0;

		VkShaderModule createShaderModule(const std::string& filename);

	};
}

