#pragma once
#include <vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

namespace core {

	struct Buffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkDeviceAddress getDeviceAddress();
		bool isCreated();
	};

	struct Image {
		VkImage image = VK_NULL_HANDLE;
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
		static void createBufferWithAlignment(const VkDeviceSize& size, const VkDeviceSize& minAlignment, Buffer& buffer, VkBufferUsageFlags usage);
		static void createBufferWithAlignment(const VkDeviceSize& size, const VkDeviceSize& minAlignment, VkBuffer& buffer, VmaAllocation& allocation, VkBufferUsageFlags usage);
		static void mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const uint32_t& offset = 0);
		static void createAndStageBuffer(const VkDeviceSize& size, const void* data, Buffer& buffer, VkBufferUsageFlags usage);
		static void createAndStageBuffer(const VkCommandBuffer& commandBuffer, const VkDeviceSize& size, const void* data, Buffer& srcBuffer, Buffer& dstBuffer, VkBufferUsageFlags usage);
		static VkDeviceAddress getBufferDeviceAddress(const VkBuffer& buffer);
		static VkDeviceAddress getBufferDeviceAddress(const Buffer& buffer);
		static void destroyBuffer(const VkBuffer& buffer, const VmaAllocation& allocation);
		static void destroyBuffer(const Buffer& buffer);

		static void createImage2D(const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, Image& image, const VkImageUsageFlags& usage);
		static void createAndStageImage2D(const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, const void* data, Image& image, VkImageUsageFlags usage);
		static void createAndStageImage2D(const VkCommandBuffer& commandBuffer, const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, const void* data, Buffer& srcBuffer, Image& dstImage, VkImageUsageFlags usage);
		static void createImageView2D(VkImage& image, VkImageView& view, const VkFormat& format, const VkImageAspectFlags aspectFlags);
		static void createSampler2D(const VkSamplerAddressMode& addressMode, const bool& enableAnisotropy, VkImage& image, VkSampler& sampler);
		static void destroyImage(const VkImage& image, const VmaAllocation& allocation);
		static void destroyImage(const Image& image);
		static void destroyImageView(const VkImageView& view);
		static void destroySampler(const VkSampler& sampler);

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
