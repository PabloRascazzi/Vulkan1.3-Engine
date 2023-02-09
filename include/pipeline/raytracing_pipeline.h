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

	private:
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

		virtual void createPipelineLayout();
		virtual void createPipeline();

	};
}