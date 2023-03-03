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
#include "vma/vk_mem_alloc.h"

#include <window.h>
#include <pipeline/pipeline.h>
#include <mesh.h>
#include <scene.h>

#include <string>
#include <vector>
#include <set>
#include <optional>

namespace core {

	const int MAX_FRAMES_IN_FLIGHT = 2;

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

	struct Image {
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VmaAllocation allocation;
	};

	class EngineContext {
	public:
		static void setup();
		static bool update();
		static void cleanup();

		static void rasterize(Pipeline& pipeline, Mesh& mesh);
		static void raytrace(Pipeline& pipeline, Scene& scene, std::vector<Image>& outImages);

		static Window* getWindow() { return &window; }
		static vk::Instance getInstance() { return instance; }
		static vk::SurfaceKHR getSurface() { return surface; }

		static vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
		static vk::Device getDevice() { return device; }
		static vk::Queue getGraphicsQueue() { return graphicsQueue; }
		static vk::Queue getPresentQueue() { return presentQueue; }

		static vk::PhysicalDeviceProperties getDeviceProperties() { return deviceProperties; }
		static vk::PhysicalDeviceRayTracingPipelinePropertiesKHR getRayTracingProperties() { return rtProperties; }
		static vk::PhysicalDeviceAccelerationStructurePropertiesKHR getAccelerationStructureProperties() { return asProperties; }

		static vk::RenderPass getRenderPass() { return renderPass; }
		static vk::SwapchainKHR getSwapChain() { return swapChain; }
		static std::vector<vk::Image> getSwapChainImages() { return swapChainImages; }
		static vk::Format getSwapChainImageFormat() { return swapChainImageFormat; }
		static vk::Extent2D getSwapChainExtent() { return swapChainExtent; }

		static void createCommandBuffer(VkCommandBuffer* buffer, uint32_t amount);

		static void exit();

		static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& alloc);
		static void createImage2D(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& alloc);
		static void createImageView2D(VkImage& image, VkFormat format, VkImageView& imageView);
		static void mapBufferData(VmaAllocation& alloc, size_t size, void* data, VkDeviceSize offset = 0);
		static void copyBufferData(VkBuffer& srcAlloc, VkBuffer& dstAlloc, size_t size);
		static void destroyBuffer(VkBuffer& buffer, VmaAllocation& alloc);
		static void destroyImage(Image& image);
		static VkDeviceAddress getBufferDeviceAddress(const VkBuffer& buffer);

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

		static vk::PhysicalDeviceProperties deviceProperties;
		static vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties;
		static vk::PhysicalDeviceAccelerationStructurePropertiesKHR asProperties;

		static vk::CommandPool commandPool;
		static VmaAllocator allocator;

		static vk::RenderPass renderPass;
		static vk::SwapchainKHR swapChain;
		static std::vector<vk::Image> swapChainImages;
		static std::vector<vk::ImageView> swapChainImageViews;
		static vk::Format swapChainImageFormat;
		static vk::Extent2D swapChainExtent;
		static std::vector<vk::Framebuffer> swapChainFramebuffers;

		static std::vector<vk::CommandBuffer> commandBuffers;
		static std::vector<vk::Semaphore> imageAvailableSemaphores;
		static std::vector<vk::Semaphore> renderFinishedSemaphores;
		static std::vector<vk::Fence> inFlightFences;

		static void setupInstanceExtensions();
		static void setupValidationLayers();
		static void createInstance();
		static void createSurface();
		static void setupDeviceExtensions();
		static void selectPhysicalDevice();
		static void createLogicalDevice();
		static void createAllocator();
		static void createSwapChain();
		static void createRenderPass();
		static void createImageViews();
		static void createCommandPool();
		static void createFramebuffers();
		static void createFrameCommandBuffers();
		static void createSyncObjects();

		static void transitionImageLayout(const VkCommandBuffer& commandBuffer, const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void transitionImageLayout(const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void recordRasterizeCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Pipeline& pipeline, Mesh& mesh);
		static void recordRaytraceCommandBuffer(const VkCommandBuffer& commandBuffer, Pipeline& pipeline, std::vector<Image>& outImages);

		static bool isDeviceSuitable(VkPhysicalDevice device);
		static QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);
		static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		static SwapChainSupport querySwapChainSupport(VkPhysicalDevice device);

		static VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
		static VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		static VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities);
	};
}
