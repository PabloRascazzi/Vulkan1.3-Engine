#pragma once
#include <descriptor_set.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	enum class PipelineType {
		PIPELINE_TYPE_NONE, 
		PIPELINE_TYPE_RASTERIZATION, 
		PIPELINE_TYPE_RAY_TRACING,
	};

	class Pipeline {
	public:
		Pipeline(VkDevice device);
		virtual void cleanup() = 0;

		vk::Pipeline getHandle() { return pipeline; }
		vk::PipelineLayout getLayout() { return layout; }
		PipelineType getType() { return type; }

	protected:
		vk::Device device;
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;
		PipelineType type;

		virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& layouts) = 0;
		virtual void createPipeline() = 0;

		VkShaderModule createShaderModule(const std::string& filename);

	};
}

