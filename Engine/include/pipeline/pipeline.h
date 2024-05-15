#pragma once
#include <descriptor_set.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	enum class PipelineType {
		PIPELINE_TYPE_NONE, 
		PIPELINE_TYPE_RASTERIZATION, 
		PIPELINE_TYPE_RAY_TRACING,
		PIPELINE_TYPE_COMPUTE,
	};

	class Pipeline {
	public:
		Pipeline(VkDevice device, const PipelineType& type);
		~Pipeline();

		VkPipeline getHandle() { return pipeline; }
		VkPipelineLayout getLayout() { return layout; }
		PipelineType getType() { return type; }

	private:
		PipelineType type;

	protected:
		VkDevice device;
		VkPipeline pipeline;
		VkPipelineLayout layout;

		virtual void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) = 0;
		virtual void createPipeline() = 0;

		VkShaderModule createShaderModule(const std::string& filename);

	};
}

