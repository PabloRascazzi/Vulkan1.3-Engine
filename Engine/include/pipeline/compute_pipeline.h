#pragma once
#include <pipeline/pipeline.h>

namespace core {

	class ComputePipeline : public Pipeline {
	public:
		ComputePipeline(VkDevice device, const std::string& shadername, const uint32_t& pushConstantSize = 0);
		ComputePipeline(VkDevice device, const std::string& shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts, const uint32_t& pushConstantSize = 0);
		~ComputePipeline() = default;

	private:
		std::string m_shadername;
		uint32_t m_pushConstantSize;

		virtual void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void CreatePipeline();

	};
}