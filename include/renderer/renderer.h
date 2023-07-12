#pragma once
#include <engine_globals.h>
#include <pipeline/pipeline.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class Renderer {
	public:
		Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue);
		~Renderer();

		virtual void cleanup() = 0;
		// TODO - virtual void render(Scene& scene) = 0;

		static VkSwapchainKHR getSwapChain() { return swapChain; }
		static std::vector<VkImage> getSwapChainImages() { return swapChainImages; }
		static VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		static VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		static void destroySwapchain();

		static uint32_t getCurrentFrame() { return currentFrame; }

	protected:
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		static uint32_t currentFrame;

		static VkSwapchainKHR swapChain;
		static VkFormat swapChainImageFormat;
		static VkExtent2D swapChainExtent;
		// Swapchain needs one Frame Buffer for each frames in flight.
		static std::vector<VkImage> swapChainImages;
		static std::vector<VkImageView> swapChainImageViews;

		static void createSwapChain();

		static VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
		static VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		static VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities);

	};
}