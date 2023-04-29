#pragma once
#include <vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

namespace core {

	struct Buffer {
		VkBuffer buffer;
		VmaAllocation allocation;
		VkDeviceAddress getDeviceAddress();
	};

	struct Image {
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VmaAllocation allocation;
	};

	class ResourceAllocator {
	public:
		static void setup(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkDevice& device, const uint32_t familyQueueIndex);
		static void cleanup();

		static void createBuffer(const VkDeviceSize& size, Buffer& buffer, VkBufferUsageFlags usage);
		static void mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const uint32_t& offset = 0);
		static void createAndStageBuffer(const VkDeviceSize& size, const void* data, Buffer& buffer, VkBufferUsageFlags usage);
		static void createAndStageBuffer(const VkCommandBuffer& commandBuffer, const VkDeviceSize& size, const void* data, Buffer& srcBuffer, Buffer& dstBuffer, VkBufferUsageFlags usage);
		static VkDeviceAddress getBufferDeviceAddress(const Buffer& buffer);
		static void destroyBuffer(Buffer& buffer);

		static void createImage2D(const VkExtent2D& extent, const VkFormat& format, Image& image, VkImageUsageFlags usage);
		static void createImageView2D(const VkFormat& format, Image& image);
		static void createSampler2D(const VkSamplerAddressMode& addressMode, const bool& enableAnisotropy, Image& image);
		static void destroyImage(const Image& buffer);

	private:
		static VkDevice device;
		static VkCommandPool commandPool;
		static VkQueue transferQueue;
		static VmaAllocator allocator;

		static void createAllocator(const VkInstance& instance, const VkPhysicalDevice physicalDevice);
		static void fetchQueue(const VkDevice& device, const uint32_t familyQueueIndex);
		static void createCommandPool(const uint32_t familyQueueIndex);
	};
}
