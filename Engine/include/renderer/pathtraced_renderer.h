#pragma once
#include <renderer/renderer.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class PathTracedRenderer : public Renderer {
	public:
		PathTracedRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<std::shared_ptr<DescriptorSet>>& globalDescSets);
		~PathTracedRenderer() = default;

	private:
		// Pipelines.
		std::unique_ptr<RayTracingPipeline> m_rtPipeline;
		std::unique_ptr<PostPipeline> m_postPipeline;
		// Descriptor buffers.
		std::vector<std::unique_ptr<Texture>> m_rtDescTextures;
		// Descriptor Sets.
		std::vector<std::shared_ptr<DescriptorSet>> m_globalDescSets;
		std::shared_ptr<DescriptorSet> m_rtDescSet;
		std::shared_ptr<DescriptorSet> m_postDescSet;

		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateDescriptorSets();
		void CreatePipeline();

		void InitDescriptorSets(Scene& scene);
		void UpdateDescriptorSets(Scene& scene);

		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}