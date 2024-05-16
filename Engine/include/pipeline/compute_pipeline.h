#pragma once
#include <pipeline/pipeline.h>

namespace core {

	class ComputePipeline : public Pipeline {
	public:
		ComputePipeline(VkDevice device, const std::string& shadername);
		ComputePipeline(VkDevice device, const std::string& shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts);
		~ComputePipeline() = default;

	private:
		std::string m_shadername;

		virtual void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void CreatePipeline();

	};
}