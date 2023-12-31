#pragma once
#include <engine_renderer.h>
#include <renderer/renderer.h>
#include <pipeline/pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class StandardRenderer : public Renderer {
	public:
		StandardRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain);
		~StandardRenderer();

		virtual void cleanup();
		virtual void render(const uint32_t currentFrame, Pipeline& pipeline, Scene& scene);

		VkRenderPass& getRenderPass() { return renderPass; }

	private:
		Swapchain& swapChain;
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// Only needs one Depth Buffer since only one draw operation is running at once.
		Image depthImage;
		VkImageView depthImageView;
		VkFormat depthImageFormat;

		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		void createDepthBuffer();
		void createRenderPass();
		void createFramebuffers();
		void createCommandBuffers();
		void createSyncObjects();

		void recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Pipeline& pipeline, Scene& scene);

	};
}