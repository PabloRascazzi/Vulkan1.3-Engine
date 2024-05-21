#include <pipeline/compute_pipeline.h>
#include <engine_globals.h>

namespace core {

	ComputePipeline::ComputePipeline(VkDevice device, const std::string& shadername, const uint32_t& pushConstantSize) :
		ComputePipeline(device, shadername, std::vector<VkDescriptorSetLayout>(), pushConstantSize) {}
	
	ComputePipeline::ComputePipeline(VkDevice device, const std::string& shadername, const std::vector<VkDescriptorSetLayout>& descSetLayouts, const uint32_t& pushConstantSize) :
		Pipeline(device, PipelineType::PIPELINE_TYPE_COMPUTE), m_shadername(shadername), m_pushConstantSize(pushConstantSize) {

		CreatePipelineLayout(descSetLayouts);
		CreatePipeline();
	}

	void ComputePipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) {
		// Pipeline push constants.
		uint32_t pushConstantRangeCount = m_pushConstantSize > 0 ? 1 : 0;
		VkPushConstantRange pushConstantRange;
		if (pushConstantRangeCount) {
			pushConstantRange.offset = 0;
			pushConstantRange.size = m_pushConstantSize;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		
		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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