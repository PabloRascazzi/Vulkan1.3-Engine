#pragma once
#include <pipeline/pipeline.h>

#include <glm/glm.hpp>
#include <vector>

namespace core {

	struct RayTracingPushConstant {
		glm::vec4 clearColor;

		static uint32_t getSize() {
			return sizeof(RayTracingPushConstant);
		}
	};

	class RayTracingPipeline  : Pipeline {
	public:
		RayTracingPipeline(VkDevice device);
		~RayTracingPipeline();
		virtual void cleanup();

		std::vector<VkRayTracingShaderGroupCreateInfoKHR>& getShaderGroups() { return shaderGroups; }
		VkDescriptorSetLayout& getDescriptorSetLayout() { return (VkDescriptorSetLayout&)descriptorSetLayout; }
		
		uint32_t getDescriptorSetsIndex() { return this->descriptorSetsIndex; }
		void setDescriptorSetsIndex(uint32_t index) { this->descriptorSetsIndex = index; }

	private:
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
		vk::DescriptorSetLayout descriptorSetLayout;
		uint32_t descriptorSetsIndex;

		virtual void createDescriptorSetLayout();
		virtual void createPipelineLayout();
		virtual void createPipeline();

	};
}