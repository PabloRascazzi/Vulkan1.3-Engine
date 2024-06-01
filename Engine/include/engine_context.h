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
		static EngineContext& GetInstance() {
			static EngineContext instance; // Guaranteed to be destroyed.
			                               // Instantiated on first use.
			return instance;
		}

		bool update();

		Window& getWindow() { return window; }
		vk::Instance getInstance() { return instance; }
		vk::SurfaceKHR getSurface() { return surface; }

		vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
		vk::Device getDevice() { return device; }
		vk::Queue getGraphicsQueue() { return graphicsQueue; }
		vk::Queue getPresentQueue() { return presentQueue; }

		PhysicalDeviceProperties getPhysicalDeviceProperties() { return physicalDeviceProperties; }
		QueueFamilyIndices getQueueFamilyIndices() { return queryQueueFamilies(physicalDevice); }
		SwapChainSupport getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }

		void createCommandBuffer(VkCommandBuffer* buffer, uint32_t amount);
		void transitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void transitionImageLayout(const VkCommandBuffer& commandBuffer, const VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void exit();

		EngineContext(EngineContext const&) = delete; // Delete public default constructor.
		void operator=(EngineContext const&) = delete; // Delete copy operator.

	private:
		EngineContext();
		~EngineContext();

		Window window;
		VkInstance instance;
		vk::SurfaceKHR surface;
		std::vector<const char*> instanceExtensions;
		std::vector<const char*> deviceExtensions;
		std::vector<const char*> layers;

		PhysicalDeviceProperties physicalDeviceProperties;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		vk::CommandPool commandPool;

		void setupInstanceExtensions();
		void setupValidationLayers();
		void createInstance();
		void createSurface();
		void setupDeviceExtensions();
		void selectPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupport querySwapChainSupport(VkPhysicalDevice device);
	};
}
