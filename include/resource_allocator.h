#pragma once
#include <vulkan/vulkan.hpp>
#include "vma/vk_mem_alloc.h"

namespace core {

	struct Buffer {
		VkBuffer buffer;
		VmaAllocation allocation;
		VkDeviceAddress getDeviceAddress();
	};

	class ResourceAllocator {
	public:
		static void setup(const VkDevice& device, const VmaAllocator& allocator , const VkCommandPool& commandPool, const VkQueue transferQueue);
		static void cleanup();

		static void createBuffer(const VkDeviceSize& size, Buffer& buffer, VkBufferUsageFlags usage);
		static void mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const VkDeviceSize& offset = 0);
		static void createAndStageBuffer(const VkDeviceSize& size, const void* data, Buffer& buffer, VkBufferUsageFlags usage);
		static void createAndStageBuffer(const VkCommandBuffer& commandBuffer, const VkDeviceSize& size, const void* data, Buffer& srcBuffer, Buffer& dstBuffer, VkBufferUsageFlags usage);
		static VkDeviceAddress getBufferDeviceAddress(const Buffer& buffer);
		static void destroyBuffer(Buffer& buffer);

	private:
		static VkDevice device;
		static VkCommandPool commandPool;
		static VkQueue transferQueue;
		static VmaAllocator allocator;

	};
}
