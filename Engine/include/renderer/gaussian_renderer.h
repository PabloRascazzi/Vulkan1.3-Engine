#pragma once
#include <renderer/renderer.h>
#include <pipeline/compute_pipeline.h>
#include <pipeline/post_pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	struct GaussianPreprocessPushConstant {
		VkDeviceAddress geomAddress;
		VkDeviceAddress keysAddress;

		static uint32_t getSize() {
			return sizeof(GaussianPreprocessPushConstant);
		}
	};

	struct GaussianRasterizePushConstant {
		glm::uvec2 resolution;
		VkDeviceAddress geomAddress;
		VkDeviceAddress keysAddress;

		static uint32_t getSize() {
			return sizeof(GaussianRasterizePushConstant);
		}
	};

	class GaussianRenderer : public Renderer {
	public:
		GaussianRenderer(VkDevice device, VkQueue computeQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<DescriptorSet*>& globalDescSets);
		~GaussianRenderer();

	private:
		// Pipelines.
		ComputePipeline* m_preprocessPipeline;
		ComputePipeline* m_rasterizePipeline;
		PostPipeline* m_postPipeline;
		// Descriptor buffers.
		std::vector<Texture*> m_gsDescTextures;
		// Descriptor Sets.
		std::vector<DescriptorSet*> m_globalDescSets;
		DescriptorSet* m_gsDescSet;
		DescriptorSet* m_postDescSet;

		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateDescriptorSets();
		void CreatePipeline();

		void InitDescriptorSets(Scene& scene);
		void UpdateDescriptorSets(Scene& scene);

		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}