#pragma once
#include <vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

namespace core {

	struct Buffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkDeviceAddress getDeviceAddress();
	};

	struct Image {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
	};

	struct AccelerationStructure {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
		VkDeviceAddress getDeviceAddress();
	};

	class ResourceAllocator {
	public:
		static void setup(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkDevice& device, const uint32_t familyQueueIndex);
		static void cleanup();

		static void createBuffer(const VkDeviceSize& size, Buffer& buffer, VkBufferUsageFlags usage);
		static void createBuffer(const VkDeviceSize& size, VkBuffer& buffer, VmaAllocation& allocation, VkBufferUsageFlags usage);
		static void mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const uint32_t& offset = 0);
		static void createAndStageBuffer(const VkDeviceSize& size, const void* data, Buffer& buffer, VkBufferUsageFlags usage);
		static void createAndStageBuffer(const VkCommandBuffer& commandBuffer, const VkDeviceSize& size, const void* data, Buffer& srcBuffer, Buffer& dstBuffer, VkBufferUsageFlags usage);
		static VkDeviceAddress getBufferDeviceAddress(const VkBuffer& buffer);
		static VkDeviceAddress getBufferDeviceAddress(const Buffer& buffer);
		static void destroyBuffer(const Buffer& buffer);
		static void destroyBuffer(const VkBuffer& buffer, const VmaAllocation& allocation);

		static void createImage2D(const VkExtent2D& extent, const VkFormat& format, Image& image, VkImageUsageFlags usage);
		static void createImageView2D(const VkFormat& format, Image& image);
		static void createSampler2D(const VkSamplerAddressMode& addressMode, const bool& enableAnisotropy, Image& image);
		static void destroyImage(const Image& buffer);

		static void createAccelerationStructure(const VkDeviceSize& size, AccelerationStructure& accelStruct, const VkAccelerationStructureTypeKHR& type);
		static void destroyAccelerationStructure(const AccelerationStructure& accelStruct);

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
