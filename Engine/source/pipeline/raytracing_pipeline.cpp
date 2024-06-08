#include <pipeline/raytracing_pipeline.h>
#include <engine_globals.h>
#include <engine_context.h>

namespace core {

	RayTracingPipeline::RayTracingPipeline(VkDevice device) : 
		RayTracingPipeline(device, std::vector<VkDescriptorSetLayout>()) {}

	RayTracingPipeline::RayTracingPipeline(VkDevice device, const std::vector<VkDescriptorSetLayout>& descSetLayouts) : 
		Pipeline(device, PipelineType::PIPELINE_TYPE_RAY_TRACING) {
		
		CreatePipelineLayout(descSetLayouts);
		CreatePipeline();
		CreateShaderBindingTable();
	}

	RayTracingPipeline::~RayTracingPipeline() {
		ResourceAllocator::destroyBuffer(m_sbt.buffer);
	}

	void RayTracingPipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts) {
		// Pipeline push constants.
		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(RayTracingPushConstant);
		pushConstant.stageFlags = RayTracingPushConstant::getShaderStageFlags();

		// Pipeline layout creation.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

		VK_CHECK_MSG(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_layout), "Could not create raytracing pipeline layout.");
	}

	void RayTracingPipeline::CreatePipeline() {
		// Create shader stages.
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkPipelineShaderStageCreateInfo genShaderStageInfo{};
		genShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		genShaderStageInfo.module = CreateShaderModule("./resource/shaders/SPIR-V/raytrace.rgen.spv");
		genShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		genShaderStageInfo.pName = "main";
		shaderStages.push_back(genShaderStageInfo);

		VkPipelineShaderStageCreateInfo missShaderStageInfo{};
		missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		missShaderStageInfo.module = CreateShaderModule("./resource/shaders/SPIR-V/raytrace.rmiss.spv");
		missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		missShaderStageInfo.pName = "main";
		shaderStages.push_back(missShaderStageInfo);

		VkPipelineShaderStageCreateInfo chitShaderStageInfo{};
		chitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		chitShaderStageInfo.module = CreateShaderModule("./resource/shaders/SPIR-V/raytrace.rchit.spv");
		chitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		chitShaderStageInfo.pName = "main";
		shaderStages.push_back(chitShaderStageInfo);

		VkPipelineShaderStageCreateInfo chitReflectShaderStageInfo{};
		chitReflectShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		chitReflectShaderStageInfo.module = CreateShaderModule("./resource/shaders/SPIR-V/raytrace_reflect.rchit.spv");
		chitReflectShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		chitReflectShaderStageInfo.pName = "main";
		//shaderStages.push_back(chitReflectShaderStageInfo);

		// Create shader groups.
		VkRayTracingShaderGroupCreateInfoKHR genShaderGroupInfo{};
		genShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		genShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		genShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		genShaderGroupInfo.generalShader = 0;
		m_shaderGroups.push_back(genShaderGroupInfo);

		VkRayTracingShaderGroupCreateInfoKHR missShaderGroupInfo{};
		missShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		missShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		missShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.generalShader = 1;
		m_shaderGroups.push_back(missShaderGroupInfo);

		VkRayTracingShaderGroupCreateInfoKHR chitShaderGroupInfo{};
		chitShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		chitShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		chitShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		chitShaderGroupInfo.closestHitShader = 2;
		chitShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		chitShaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		m_shaderGroups.push_back(chitShaderGroupInfo);

		VkRayTracingShaderGroupCreateInfoKHR chitReflectShaderGroupInfo{};
		chitReflectShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		chitReflectShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		chitReflectShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		chitReflectShaderGroupInfo.closestHitShader = 3;
		chitReflectShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		chitReflectShaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		//m_shaderGroups.push_back(chitReflectShaderGroupInfo);

		// Create pipeline.
		VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.groupCount = static_cast<uint32_t>(m_shaderGroups.size());
		pipelineInfo.pGroups = m_shaderGroups.data();
		pipelineInfo.maxPipelineRayRecursionDepth = 2; 
		pipelineInfo.layout = m_layout;
		
		VK_CHECK_MSG(vkCreateRayTracingPipelinesKHR(m_device, {}, {}, 1, &pipelineInfo, nullptr, &m_pipeline), "Failed to create ray-tracing pipeline.");

		// Pipeline creation cleanup.
		vkDestroyShaderModule(m_device, genShaderStageInfo.module, nullptr);
		vkDestroyShaderModule(m_device, missShaderStageInfo.module, nullptr);
		vkDestroyShaderModule(m_device, chitShaderStageInfo.module, nullptr);
		vkDestroyShaderModule(m_device, chitReflectShaderStageInfo.module, nullptr);
	}

	void RayTracingPipeline::CreateShaderBindingTable() {
		uint32_t missCount = 1;
		uint32_t hitCount = 1;
		uint32_t handleCount = 1 + missCount + hitCount;
		uint32_t handleSize = EngineContext::GetInstance().getPhysicalDeviceProperties().raytracingProperties.shaderGroupHandleSize;

		// Align all group handles.
		uint32_t handleSizeAligned = alignUp(handleSize, EngineContext::GetInstance().getPhysicalDeviceProperties().raytracingProperties.shaderGroupHandleAlignment);
		m_sbt.rgenRegion.stride = alignUp(handleSizeAligned, EngineContext::GetInstance().getPhysicalDeviceProperties().raytracingProperties.shaderGroupBaseAlignment);
		m_sbt.rgenRegion.size = m_sbt.rgenRegion.stride; // Ray generation size must be equal to ray generation stride.
		m_sbt.missRegion.stride = handleSizeAligned;
		m_sbt.missRegion.size = alignUp(missCount * handleSizeAligned, EngineContext::GetInstance().getPhysicalDeviceProperties().raytracingProperties.shaderGroupBaseAlignment);
		m_sbt.hitRegion.stride = handleSizeAligned;
		m_sbt.hitRegion.size = alignUp(hitCount * handleSizeAligned, EngineContext::GetInstance().getPhysicalDeviceProperties().raytracingProperties.shaderGroupBaseAlignment);

	    // Get the shader group handles.
		uint32_t dataSize = handleCount * handleSize;
		std::vector<uint8_t> handles(dataSize);
		if (vkGetRayTracingShaderGroupHandlesKHR(m_device, m_pipeline, 0, handleCount, dataSize, handles.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to get shader group handles.");
		}

		// Create SBT buffer.
		VkDeviceSize sbtSize = m_sbt.rgenRegion.size + m_sbt.missRegion.size + m_sbt.hitRegion.size + m_sbt.callRegion.size;
		VkBufferUsageFlags stbBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		ResourceAllocator::createBuffer(sbtSize, m_sbt.buffer, stbBufferUsage, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		Debugger::setObjectName(m_sbt.buffer.buffer, "SBT");
		
		// Find the SBT addresses of each group.
		VkDeviceAddress sbtAddress = m_sbt.buffer.getDeviceAddress();
		m_sbt.rgenRegion.deviceAddress = sbtAddress;
		m_sbt.missRegion.deviceAddress = sbtAddress + m_sbt.rgenRegion.size;
		m_sbt.hitRegion.deviceAddress  = sbtAddress + m_sbt.rgenRegion.size + m_sbt.missRegion.size;

		// Helper to retrieve the handle data
		auto getHandle = [&] (int i) { return handles.data() + i * handleSize; };

		// Map SBT buffer with SBT handles.
		uint32_t offset = 0;
		uint32_t handleIndex = 0; 

		// Raygen
		offset = 0;
		ResourceAllocator::mapDataToBuffer(m_sbt.buffer, handleSize, getHandle(handleIndex++), offset);
		// Miss
		offset = static_cast<uint32_t>(m_sbt.rgenRegion.size);
		for (uint32_t i = 0; i < missCount; i++) {
			ResourceAllocator::mapDataToBuffer(m_sbt.buffer, handleSize, getHandle(handleIndex++), offset);
			offset += static_cast<uint32_t>(m_sbt.missRegion.stride);
		}
		// Hit
		offset = static_cast<uint32_t>(m_sbt.rgenRegion.size + m_sbt.missRegion.size);
		for (uint32_t i = 0; i < hitCount; i++) {
			ResourceAllocator::mapDataToBuffer(m_sbt.buffer, handleSize, getHandle(handleIndex++), offset);
			offset += static_cast<uint32_t>(m_sbt.hitRegion.stride);
		}
	}
}