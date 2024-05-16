#include <pipeline/compute_pipeline.h>
#include <engine_globals.h>

namespace core {

	ComputePipeline::ComputePipeline(VkDevice device, const std::string& shadername) :
		ComputePipeline(device, shadername, std::vector<VkDescriptorSetLayout>()) {}
	
	ComputePipeline::ComputePipeline(VkDevice device, const std::string& shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts) :
		Pipeline(device, PipelineType::PIPELINE_TYPE_COMPUTE), m_shadername(shadername) {

		CreatePipelineLayout(descSetLayouts);
		CreatePipeline();
	}

	void ComputePipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) {
		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VK_CHECK_MSG(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_layout), "Could not create compute pipeline layout.");
	}

	void ComputePipeline::CreatePipeline() {
		// Shader stage creation.
		VkShaderModule shaderModule = CreateShaderModule("./resource/shaders/SPIR-V/" + m_shadername + ".comp.spv");

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		
		// Compute pipeline creation.
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = shaderStageInfo;
		pipelineInfo.layout = m_layout;

		VK_CHECK_MSG(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline), "Could not create compute pipeline.");

		// Pipeline creation cleanup.
		vkDestroyShaderModule(m_device, shaderModule, nullptr);
	}
}