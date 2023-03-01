#include <pipeline/raytracing_pipeline.h>
#include <engine_context.h>

namespace core {

	RayTracingPipeline::RayTracingPipeline(VkDevice device) : Pipeline(device) {
		this->type = PipelineType::PIPELINE_TYPE_RAY_TRACING;
		
		createDescriptorSetLayout();
		createPipelineLayout();
		createPipeline();
		createShaderBindingTable();
	}

	RayTracingPipeline::~RayTracingPipeline() {
		cleanup();
	}

	void RayTracingPipeline::cleanup() {
		EngineContext::destroyBuffer(sbt.buffer, sbt.alloc);
		device.destroyPipeline(pipeline);
		device.destroyPipelineLayout(layout);
		device.destroyDescriptorSetLayout(descriptorSetLayout);
	}

	void RayTracingPipeline::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding tlasBinding{};
		tlasBinding.binding = 0;
		tlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		tlasBinding.descriptorCount = 1;
		tlasBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding outImageBinding{};
		outImageBinding.binding = 1;
		outImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		outImageBinding.descriptorCount = 1;
		outImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding bindings[] = { tlasBinding, outImageBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 2;
		layoutInfo.pBindings = bindings;

		if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, (VkDescriptorSetLayout*)&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}
	}

	void RayTracingPipeline::createPipelineLayout() {
		// Pipeline push constants.
		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(RayTracingPushConstant);
		pushConstant.stageFlags = RayTracingPushConstant::getShaderStageFlags();

		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)&descriptorSetLayout;
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

	void RayTracingPipeline::createShaderBindingTable() {
		uint32_t missCount = 1;
		uint32_t hitCount = 1;
		uint32_t handleCount = 1 + missCount + hitCount;
		uint32_t handleSize = EngineContext::getRayTracingProperties().shaderGroupHandleSize;

		// Align all group handles.
		uint32_t handleSizeAligned = alignUp(handleSize, EngineContext::getRayTracingProperties().shaderGroupHandleAlignment);
		sbt.rgenRegion.stride = alignUp(handleSizeAligned, EngineContext::getRayTracingProperties().shaderGroupBaseAlignment);
		sbt.rgenRegion.size = sbt.rgenRegion.stride; // Ray generation size must be equal to ray generation stride.
		sbt.missRegion.stride = handleSizeAligned;
		sbt.missRegion.size = alignUp(missCount * handleSizeAligned, EngineContext::getRayTracingProperties().shaderGroupBaseAlignment);
		sbt.hitRegion.stride = handleSizeAligned;
		sbt.hitRegion.size = alignUp(hitCount * handleSizeAligned, EngineContext::getRayTracingProperties().shaderGroupBaseAlignment);

		// Get the shader group handles.
		uint32_t dataSize = handleCount * handleSize;
		std::vector<uint8_t> handles(dataSize);
		if (vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, 0, handleCount, dataSize, handles.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to get shader group handles.");
		}

		// Create SBT buffer.
		VkDeviceSize sbtSize = sbt.rgenRegion.size + sbt.missRegion.size + sbt.hitRegion.size + sbt.callRegion.size;
		VkBufferUsageFlags stbBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		EngineContext::createBuffer(sbtSize, stbBufferUsage, sbt.buffer, sbt.alloc);
		
		// Find the SBT addresses of each group.
		VkDeviceAddress sbtAddress = EngineContext::getBufferDeviceAddress(sbt.buffer);
		sbt.rgenRegion.deviceAddress = sbtAddress;
		sbt.missRegion.deviceAddress = sbtAddress + sbt.rgenRegion.size;
		sbt.hitRegion.deviceAddress  = sbtAddress + sbt.rgenRegion.size + sbt.missRegion.size;

		// Helper to retrieve the handle data
		auto getHandle = [&] (int i) { return handles.data() + i * handleSize; };

		// Map SBT buffer with SBT handles.
		uint32_t handleIndex = 0; 
		size_t offset = 0;
		EngineContext::mapBufferData(sbt.alloc, handleSize, getHandle(handleIndex++), offset); // Raygen
		offset += sbt.rgenRegion.size;
		for (uint32_t i = 0; i < missCount; i++) {
			EngineContext::mapBufferData(sbt.alloc, handleSize, getHandle(handleIndex++), offset); // Miss
			offset += sbt.missRegion.stride;
		}
		offset = 0 + sbt.rgenRegion.size + sbt.missRegion.size;
		for (uint32_t i = 0; i < hitCount; i++) {
			EngineContext::mapBufferData(sbt.alloc, handleSize, getHandle(handleIndex++), offset+=sbt.rgenRegion.size); // Hit
			offset += sbt.hitRegion.stride;
		}
	}
}