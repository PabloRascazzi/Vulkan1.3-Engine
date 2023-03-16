#include <resource_allocator.h>

#define VK_CHECK_MSG(func, msg) if(func != VK_SUCCESS) { throw std::runtime_error(msg); }
#define VK_CHECK(func) VK_CHECK_MSG(func, "Vulkan error detected at line " + std::to_string(__LINE__) + " .");

namespace core {

    VkDevice ResourceAllocator::device;
	VkCommandPool ResourceAllocator::commandPool;
	VkQueue ResourceAllocator::transferQueue;
	VmaAllocator ResourceAllocator::allocator;

    VkDeviceAddress Buffer::getDeviceAddress() {
        return ResourceAllocator::getBufferDeviceAddress(*this);
    }

    void ResourceAllocator::setup(const VkDevice& device, const VmaAllocator& allocator, const VkCommandPool& commandPool, const VkQueue transferQueue) {
        ResourceAllocator::device = device;
        ResourceAllocator::allocator = allocator;
        ResourceAllocator::commandPool = commandPool;
        ResourceAllocator::transferQueue = transferQueue;
    }

    void ResourceAllocator::cleanup() {
        // Nothing.
    }

    void ResourceAllocator::createBuffer(const VkDeviceSize& size, Buffer& buffer, VkBufferUsageFlags usage) {
        VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VmaAllocationInfo allocInfo;
        if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.buffer, &buffer.allocation, &allocInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer.");
        }
    }

    void ResourceAllocator::mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const VkDeviceSize& offset) {
        VkDeviceSize* location;
        vmaMapMemory(allocator, buffer.allocation, (void**)&location);
        memcpy(location+offset, data, size);
        vmaUnmapMemory(allocator, buffer.allocation);
    }

    void ResourceAllocator::createAndStageBuffer(const VkCommandBuffer& commandBuffer, const VkDeviceSize& size, const void* data, Buffer& srcBuffer, Buffer& dstBuffer, VkBufferUsageFlags usage) {
        // Create source buffer.
        createBuffer(size, srcBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        
        // Map data to source buffer.
        mapDataToBuffer(srcBuffer, size, data);

        // Create destination buffer.
        createBuffer(size, dstBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage);

        // Copy data from source buffer to destination buffer.
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
    }

    void ResourceAllocator::createAndStageBuffer(const VkDeviceSize& size, const void* data, Buffer& buffer, VkBufferUsageFlags usage) {
        // Create new command buffer.
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

        // Create memory transfer semaphore.
        VkSemaphoreTypeCreateInfo timelineSemaphoreInfo{};
        timelineSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineSemaphoreInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineSemaphoreInfo.initialValue = 0;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &timelineSemaphoreInfo;
        
        VkSemaphore memoryTransferComplete;
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &memoryTransferComplete));

        // Record command buffer.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        Buffer srcBuffer;
        createAndStageBuffer(commandBuffer, size, data, srcBuffer, buffer, usage);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));

        const uint64_t signalValue = 1;
        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.signalSemaphoreValueCount = 1;
        timelineInfo.pSignalSemaphoreValues = &signalValue;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &memoryTransferComplete;
        submitInfo.pNext = &timelineInfo;

        VK_CHECK(vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE));

        // Wait for memory transfer to complete.
        const uint64_t waitValue = 1;
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &memoryTransferComplete;
        waitInfo.pValues = &waitValue;
        VK_CHECK(vkWaitSemaphores(device, &waitInfo, UINT64_MAX));

        // Destroy staging buffer, command buffer, and semaphore.
        destroyBuffer(srcBuffer);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroySemaphore(device, memoryTransferComplete, nullptr);
    }

    VkDeviceAddress ResourceAllocator::getBufferDeviceAddress(const Buffer& buffer) {
        VkBufferDeviceAddressInfo addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.buffer = buffer.buffer;

        return vkGetBufferDeviceAddress(device, &addressInfo);
    }

	void ResourceAllocator::destroyBuffer(Buffer& buffer) {
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    }
}