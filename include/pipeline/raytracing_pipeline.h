#pragma once
#include <pipeline/pipeline.h>
#include "vma/vk_mem_alloc.h"

#include <glm/glm.hpp>
#include <vector>

#include <resource_allocator.h>

namespace core {

	template <class integral>
	constexpr integral alignUp(integral x, size_t a) noexcept {
		return integral((x + (integral(a) - 1)) & ~integral(a - 1));
	}

	struct RayTracingPushConstant {
		glm::vec4 clearColor;

		static uint32_t getSize() {
			return sizeof(RayTracingPushConstant);
		}

		static VkShaderStageFlags getShaderStageFlags() {
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		}
	};

	struct SBTWrapper {
		Buffer buffer;
		VkStridedDeviceAddressRegionKHR rgenRegion{};
		VkStridedDeviceAddressRegionKHR missRegion{};
		VkStridedDeviceAddressRegionKHR hitRegion{};
		VkStridedDeviceAddressRegionKHR callRegion{};
	};

	class RayTracingPipeline  : Pipeline {
	public:
		RayTracingPipeline(VkDevice device);
		RayTracingPipeline(VkDevice device, std::vector<DescriptorSet*> descriptorSets);
		~RayTracingPipeline();
		virtual void cleanup();

		std::vector<VkRayTracingShaderGroupCreateInfoKHR>& getShaderGroups() { return shaderGroups; }
		SBTWrapper& getSBT() { return sbt; }

	private:
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
		uint32_t descriptorSetsIndex;
		SBTWrapper sbt;

		virtual void createPipelineLayout();
		virtual void createPipeline();
		void createShaderBindingTable();

	};
}