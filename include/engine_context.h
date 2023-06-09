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
#include "vma/vk_mem_alloc.h"

#include <window.h>
#include <pipeline/pipeline.h>
#include <mesh.h>
#include <camera.h>
#include <scene.h>

#include <string>
#include <vector>
#include <set>
#include <optional>

#include <debugger.h>

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

	struct PhysicalDeviceProperties {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingProperties;
		VkPhysicalDeviceAccelerationStructurePropertiesKHR accelStructProperties;
	};

	class EngineContext {
	public:
		static void setup();
		static bool update();
		static void cleanup();

		static Window& getWindow() { return window; }
		static vk::Instance getInstance() { return instance; }
		static vk::SurfaceKHR getSurface() { return surface; }

		static vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
		static vk::Device getDevice() { return device; }
		static vk::Queue getGraphicsQueue() { return graphicsQueue; }
		static vk::Queue getPresentQueue() { return presentQueue; }

		static PhysicalDeviceProperties getPhysicalDeviceProperties() { return physicalDeviceProperties; }
		static QueueFamilyIndices getQueueFamilyIndices() { return queryQueueFamilies(physicalDevice); }
		static SwapChainSupport getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }

		static void createCommandBuffer(VkCommandBuffer* buffer, uint32_t amount);
		static void transitionImageLayout(const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void transitionImageLayout(const VkCommandBuffer& commandBuffer, const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);

		static void exit();

	private:
		static Window window;
		static VkInstance instance;
		static vk::SurfaceKHR surface;
		static std::vector<const char*> instanceExtensions;
		static std::vector<const char*> deviceExtensions;
		static std::vector<const char*> layers;

		static PhysicalDeviceProperties physicalDeviceProperties;
		static vk::PhysicalDevice physicalDevice;
		static vk::Device device;
		static vk::Queue graphicsQueue;
		static vk::Queue presentQueue;

		static vk::CommandPool commandPool;

		static void setupInstanceExtensions();
		static void setupValidationLayers();
		static void createInstance();
		static void createSurface();
		static void setupDeviceExtensions();
		static void selectPhysicalDevice();
		static void createLogicalDevice();
		static void createCommandPool();

		static bool isDeviceSuitable(VkPhysicalDevice device);
		static QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);
		static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		static SwapChainSupport querySwapChainSupport(VkPhysicalDevice device);
	};
}
