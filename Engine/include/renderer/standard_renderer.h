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
		StandardRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain, std::vector<DescriptorSet*> globalDescSets);
		~StandardRenderer();

		virtual void render(const uint32_t currentFrame, Scene& scene);

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

		// Pipelines.
		Pipeline* pipeline;
		// Descriptor Sets.
		std::vector<DescriptorSet*> globalDescSets;

		void createDepthBuffer();
		void createRenderPass();
		void createFramebuffers();
		void createCommandBuffers();
		void createSyncObjects();

		void createPipeline(VkDevice device);
		void createDescriptorSets();
		void initDescriptorSets();
		void updateDescriptorSets();

		void recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}