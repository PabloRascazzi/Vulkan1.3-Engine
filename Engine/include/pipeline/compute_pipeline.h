#pragma once
#include <pipeline/pipeline.h>

namespace core {

	class ComputePipeline : public Pipeline {
	public:
		ComputePipeline(VkDevice device, const std::string& filename);
		ComputePipeline(VkDevice device, const std::string& filename, const std::vector<VkDescriptorSetLayout>& descSetLayouts);
		~ComputePipeline() = default;

	private:
		std::string filename;

		virtual void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts);
		virtual void createPipeline();

	};
}