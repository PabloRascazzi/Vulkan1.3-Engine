#pragma once

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.hpp>
#define ENGINE_GRAPHICS_API_VERSION VK_API_VERSION_1_3

#include <window.h>

#include <string>
#include <vector>
#include <set>
#include <optional>

namespace core {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
		std::vector<uint32_t> toVector() {
			std::set<uint32_t> unique = {graphicsFamily.value(), presentFamily.value()};
			return std::vector<uint32_t>(unique.begin(), unique.end());
		}
	};

	struct SwapChainSupport {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		bool isAdequate() {
			return !formats.empty() && !presentModes.empty();
		}
	};

	class EngineContext {
	public:
		static void setup();
		static bool update();
		static void cleanup();

		static Window* getWindow() { return &window; }
		static vk::Instance getInstance() { return instance; }
		static vk::SurfaceKHR getSurface() { return surface; }

		static vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
		static vk::Device getDevice() { return device; }
		static vk::Queue getGraphicsQueue() { return graphicsQueue; }
		static vk::Queue getPresentQueue() { return presentQueue; }

		static vk::RenderPass getRenderPass() { return renderPass; }
		static vk::SwapchainKHR getSwapChain() { return swapChain; }
		static std::vector<vk::Image> getSwapChainImages() { return swapChainImages; }
		static vk::Format getSwapChainImageFormat() { return swapChainImageFormat; }
		static vk::Extent2D getSwapChainExtent() { return swapChainExtent; }

		static void exit();

	private:
		static Window window;
		static vk::Instance instance;
		static vk::SurfaceKHR surface;
		static std::vector<const char*> instanceExtensions;
		static std::vector<const char*> deviceExtensions;
		static std::vector<const char*> layers;

		static vk::PhysicalDevice physicalDevice;
		static vk::Device device;
		static vk::Queue graphicsQueue;
		static vk::Queue presentQueue;

		static vk::RenderPass renderPass;
		static vk::SwapchainKHR swapChain;
		static std::vector<vk::Image> swapChainImages;
		static std::vector<vk::ImageView> swapChainImageViews;
		static vk::Format swapChainImageFormat;
		static vk::Extent2D swapChainExtent;

		static void setupInstanceExtensions();
		static void setupValidationLayers();
		static void createInstance();
		static void createSurface();
		static void setupDeviceExtensions();
		static void selectPhysicalDevice();
		static void createLogicalDevice();
		static void createSwapChain();
		static void createRenderPass();
		static void createImageViews();

		static bool isDeviceSuitable(VkPhysicalDevice device);
		static QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);
		static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		static SwapChainSupport querySwapChainSupport(VkPhysicalDevice device);

		static VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
		static VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		static VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities);
	};
}
