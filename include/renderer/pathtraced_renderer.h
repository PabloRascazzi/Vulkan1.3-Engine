#pragma once
#include <engine_renderer.h>
#include <renderer/renderer.h>
#include <pipeline/pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class PathTracedRenderer : public Renderer {
	public:
		PathTracedRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain, std::vector<DescriptorSet*> globalDescSets);
		~PathTracedRenderer();

		virtual void cleanup();
		virtual void render(const uint32_t currentFrame, Scene& scene);

		VkRenderPass& getRenderPass() { return renderPass; }

	private:
		Swapchain& swapChain;
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		// Pipelines.
		Pipeline* rtPipeline;
		Pipeline* postPipeline;
		// Descriptor buffers.
		std::vector<Texture*> rtDescTextures;
		// Descriptor Sets.
		DescriptorSet* rtDescSet;
		DescriptorSet* postDescSet;

		bool firstRender;

		void createRenderPass();
		void createFramebuffers();
		void createCommandBuffers();
		void createSyncObjects();

		void createPipeline(VkDevice device, std::vector<DescriptorSet*> globalDescSets);
		void createDescriptorSets();
		void initDescriptorSets(Scene& scene);
		void updateDescriptorSets();

		void recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene& scene);

	};
}