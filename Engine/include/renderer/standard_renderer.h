#pragma once
#include <renderer/renderer.h>
#include <pipeline/standard_pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class StandardRenderer : public Renderer {
	public:
		StandardRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<DescriptorSet*> globalDescSets);
		~StandardRenderer();

	private:
		// Only needs one Depth Buffer since only one draw operation is running at once.
		Image m_depthImage;
		VkImageView m_depthImageView;
		VkFormat m_depthImageFormat;

		// Pipelines.
		StandardPipeline* m_pipeline;
		// Descriptor Sets.
		std::vector<DescriptorSet*> m_globalDescSets;

		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateDescriptorSets();
		void CreatePipeline();

		void InitDescriptorSets(Scene& scene);
		void UpdateDescriptorSets(Scene& scene);

		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}