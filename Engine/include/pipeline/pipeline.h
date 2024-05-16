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

		VkPipeline GetHandle() { return m_pipeline; }
		VkPipelineLayout GetLayout() { return m_layout; }
		PipelineType GetType() { return m_type; }

	private:
		PipelineType m_type;

	protected:
		VkDevice m_device;
		VkPipeline m_pipeline;
		VkPipelineLayout m_layout;

		virtual void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) = 0;
		virtual void CreatePipeline() = 0;

		VkShaderModule CreateShaderModule(const std::string& filename);

	};
}

