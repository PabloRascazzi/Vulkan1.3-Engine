#include <pipeline/compute_pipeline.h>
#include <engine_globals.h>

namespace core {

	ComputePipeline::ComputePipeline(VkDevice device, const std::string& filename) :
		ComputePipeline(device, filename, std::vector<VkDescriptorSetLayout>()) {}
	
	ComputePipeline::ComputePipeline(VkDevice device, const std::string& filename, const std::vector<VkDescriptorSetLayout>& descSetLayouts) : 
		Pipeline(device, PipelineType::PIPELINE_TYPE_COMPUTE) {
		
		this->filename = filename;

		createPipelineLayout(descSetLayouts);
		createPipeline();
	}

	void ComputePipeline::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) {
		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VK_CHECK_MSG(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout), "Could not create compute pipeline layout.");
	}

	void ComputePipeline::createPipeline() {
		// Shader stage creation.
		VkShaderModule shaderModule = createShaderModule("./resource/shaders/SPIR-V/" + filename + ".comp.spv");

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		
		// Compute pipeline creation.
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = shaderStageInfo;
		pipelineInfo.layout = layout;

		VK_CHECK_MSG(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Could not create compute pipeline.");

		// Pipeline creation cleanup.
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}
}