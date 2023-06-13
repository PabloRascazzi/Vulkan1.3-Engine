#pragma once
#include <pipeline/pipeline.h>
#include <resource_allocator.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class Renderer {
	public:
		static void setup(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue);
		static void cleanup();

		static void rasterize(Pipeline& pipeline, Scene& scene);
		static void raytrace(Pipeline& rtPipeline, Pipeline& postPipeline, Scene& scene);

		static VkRenderPass& getRenderPass() { return renderPass; }
		static VkSwapchainKHR getSwapChain() { return swapChain; }
		static std::vector<VkImage> getSwapChainImages() { return swapChainImages; }
		static VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		static VkExtent2D getSwapChainExtent() { return swapChainExtent; }

		static uint32_t getCurrentFrame() { return currentFrame; }

	private:
		static VkDevice device;
		static VkQueue graphicsQueue;
		static VkQueue presentQueue;

		static VkRenderPass renderPass;
		static VkSwapchainKHR swapChain;
		static std::vector<VkImage> swapChainImages;
		static std::vector<VkImageView> swapChainImageViews;
		static VkFormat swapChainImageFormat;
		static VkExtent2D swapChainExtent;
		static std::vector<VkFramebuffer> swapChainFramebuffers;

		static std::vector<VkCommandBuffer> commandBuffers;
		static std::vector<VkSemaphore> imageAvailableSemaphores;
		static std::vector<VkSemaphore> renderFinishedSemaphores;
		static std::vector<VkFence> inFlightFences;

		static uint32_t currentFrame;

		static void createSwapChain();
		static void createRenderPass();
		static void createImageViews();
		static void createFramebuffers();
		static void createFrameCommandBuffers();
		static void createSyncObjects();

		static void recordRasterizeCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Pipeline& pipeline, Scene& scene);
		static void recordRaytraceCommandBuffer(const VkCommandBuffer& commandBuffer, Pipeline& rtPipeline, Pipeline& postPipeline, uint32_t imageIndex);

		static VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
		static VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		static VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities);

	};
}
