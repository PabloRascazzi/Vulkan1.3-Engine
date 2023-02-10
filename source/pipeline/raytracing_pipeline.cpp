#include <pipeline/raytracing_pipeline.h>

namespace core {

	RayTracingPipeline::RayTracingPipeline(VkDevice device) : Pipeline(device) {
		this->type = PipelineType::PIPELINE_TYPE_RAY_TRACING;
		
		createPipelineLayout();
		createPipeline();
	}

	RayTracingPipeline::~RayTracingPipeline() {
		cleanup();
	}

	void RayTracingPipeline::cleanup() {
		device.destroyPipeline(pipeline);
		device.destroyPipelineLayout(layout);
	}

	void RayTracingPipeline::createPipelineLayout() {
		// Pipeline push constants.
		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(RayTracingPushConstant);
		pushConstant.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, (VkPipelineLayout*)&layout) != VK_SUCCESS) {
			throw std::runtime_error("Could not create pipeline layout.");
		}
	}

	void RayTracingPipeline::createPipeline() {
		// Create shader stages.
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkPipelineShaderStageCreateInfo genShaderStageInfo{};
		genShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		genShaderStageInfo.module = createShaderModule("./resource/shaders/SPIR-V/raytrace.rgen.spv");
		genShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		genShaderStageInfo.pName = "main";
		shaderStages.push_back(genShaderStageInfo);

		VkPipelineShaderStageCreateInfo missShaderStageInfo{};
		missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		missShaderStageInfo.module = createShaderModule("./resource/shaders/SPIR-V/raytrace.rmiss.spv");
		missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		missShaderStageInfo.pName = "main";
		shaderStages.push_back(missShaderStageInfo);

		VkPipelineShaderStageCreateInfo chitShaderStageInfo{};
		chitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		chitShaderStageInfo.module = createShaderModule("./resource/shaders/SPIR-V/raytrace.rchit.spv");
		chitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		chitShaderStageInfo.pName = "main";
		shaderStages.push_back(chitShaderStageInfo);

		// Create shader groups.
		VkRayTracingShaderGroupCreateInfoKHR genShaderGroupInfo{};
		genShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		genShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		genShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.generalShader = 0;
		shaderGroups.push_back(genShaderGroupInfo);

		VkRayTracingShaderGroupCreateInfoKHR missShaderGroupInfo{};
		missShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		missShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		missShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.generalShader = 1;
		shaderGroups.push_back(missShaderGroupInfo);

		VkRayTracingShaderGroupCreateInfoKHR chitShaderGroupInfo{};
		chitShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		chitShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		chitShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		chitShaderGroupInfo.closestHitShader = 2;
		chitShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		chitShaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(chitShaderGroupInfo);

		// Create pipeline.
		VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
		pipelineInfo.pGroups = shaderGroups.data();
		pipelineInfo.maxPipelineRayRecursionDepth = 1; 
		pipelineInfo.layout = layout;
		
		if (vkCreateRayTracingPipelinesKHR(device, {}, {}, 1, &pipelineInfo, nullptr, (VkPipeline*)&pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create ray-tracing pipeline.");
		}

		// Pipeline creation cleanup.
		device.destroyShaderModule(genShaderStageInfo.module);
		device.destroyShaderModule(missShaderStageInfo.module);
		device.destroyShaderModule(chitShaderStageInfo.module);
	}
}