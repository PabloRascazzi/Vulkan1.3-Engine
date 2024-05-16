#pragma once
#include <engine_renderer.h>
#include <renderer/renderer.h>
#include <pipeline/pipeline.h>
#include <pipeline/compute_pipeline.h>
#include <pipeline/post_pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class GaussianRenderer : public Renderer {
	public:
		GaussianRenderer(VkDevice device, VkQueue computeQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<DescriptorSet*>& globalDescSets);
		~GaussianRenderer();

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
		ComputePipeline* gsPipeline;
		PostPipeline* postPipeline;
		// Descriptor buffers.
		std::vector<Texture*> gsDescTextures;
		// Descriptor Sets.
		std::vector<DescriptorSet*> globalDescSets;
		DescriptorSet* gsDescSet;
		DescriptorSet* postDescSet;

		bool firstRender;

		void createRenderPass();
		void createFramebuffers();
		void createCommandBuffers();
		void createSyncObjects();

		void createPipeline(VkDevice device);
		void createDescriptorSets();
		void initDescriptorSets(Scene& scene);
		void updateDescriptorSets();

		void recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene);

	};
}