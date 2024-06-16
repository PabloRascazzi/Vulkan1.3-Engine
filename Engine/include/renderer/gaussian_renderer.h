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
		VkDeviceAddress bufferAddress;
		glm::uvec2 resolution;
		uint32_t numGaussians;

		static uint32_t getSize() {
			return sizeof(GaussianPreprocessPushConstant);
		}
	};

	struct GaussianRasterizePushConstant {
		VkDeviceAddress bufferAddress;
		glm::uvec2 resolution;
		uint32_t numGaussians;

		static uint32_t getSize() {
			return sizeof(GaussianRasterizePushConstant);
		}
	};

	class GaussianRenderer : public Renderer {
	public:
		GaussianRenderer(VkDevice device, VkQueue computeQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<std::shared_ptr<DescriptorSet>>& globalDescSets);
		~GaussianRenderer();

	private:
		// Pipelines.
		std::unique_ptr<ComputePipeline> m_preprocessPipeline;
		std::unique_ptr<ComputePipeline> m_rasterizePipeline;
		std::unique_ptr<PostPipeline> m_postPipeline;
		// Descriptor buffers.
		std::vector<std::unique_ptr<Texture>> m_gsDescTextures;
		// Descriptor Sets.
		std::vector<std::shared_ptr<DescriptorSet>> m_globalDescSets;
		std::shared_ptr<DescriptorSet> m_gsDescSet;
		std::shared_ptr<DescriptorSet> m_postDescSet;

		Buffer m_geomBuffer; // Gaussian geometry.
		Buffer m_procBuffer; // Preprocessed Gaussian geometry.
		Buffer m_keysBuffer; // Gaussian keys (Id + Depth).

		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateDescriptorSets();
		void CreatePipeline();

		void InitDescriptorSets(Scene& scene);
		void UpdateDescriptorSets(Scene& scene);

		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}