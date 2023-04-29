#include <resource_allocator.h>
#include <engine_context.h>

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

    void ResourceAllocator::setup(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkDevice& device, const uint32_t familyQueueIndex) {
        ResourceAllocator::device = device;
        createAllocator(instance, physicalDevice);
        fetchQueue(device, familyQueueIndex);
        createCommandPool(familyQueueIndex);
    }

    void ResourceAllocator::cleanup() {
        vkDestroyCommandPool(device, commandPool, nullptr);
        vmaDestroyAllocator(allocator);
    }

    void ResourceAllocator::createAllocator(const VkInstance& instance, const VkPhysicalDevice physicalDevice) {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = ENGINE_GRAPHICS_API_VERSION;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.pVulkanFunctions = nullptr; // optional
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        VK_CHECK_MSG(vmaCreateAllocator(&allocatorInfo, &allocator), "Failed to create memory allocator.");
    }

    void ResourceAllocator::fetchQueue(const VkDevice& device, const uint32_t familyQueueIndex) {
        vkGetDeviceQueue(device, familyQueueIndex, 0, &transferQueue);
    }

    void ResourceAllocator::createCommandPool(const uint32_t familyQueueIndex) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = familyQueueIndex;

        VK_CHECK_MSG(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "Failed to create resource allocator command pool.");
    }

    //***************************************************************************************//
    //                                   Buffer Allocation                                   //
    //***************************************************************************************//

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
        VK_CHECK(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.buffer, &buffer.allocation, &allocInfo));
    }

    void ResourceAllocator::mapDataToBuffer(const Buffer& buffer, const VkDeviceSize& size, const void* data, const uint32_t& offset) {
        uint8_t* location;
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

    //***************************************************************************************//
    //                                   Image Allocation                                    //
    //***************************************************************************************//

    void ResourceAllocator::createImage2D(const VkExtent2D& extent, const VkFormat& format, Image& image, VkImageUsageFlags usage) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        imageAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        VK_CHECK(vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &image.image, &image.allocation, nullptr));
    }

    void ResourceAllocator::createImageView2D(const VkFormat& format, Image& image) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = format;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &image.view));
    }

    void ResourceAllocator::createSampler2D(const VkSamplerAddressMode& addressMode, const bool& enableAnisotropy, Image& image) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = enableAnisotropy;
        samplerInfo.maxAnisotropy = EngineContext::getDeviceProperties().limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &image.sampler));
    }

    void ResourceAllocator::destroyImage(const Image& image) {
        vmaDestroyImage(allocator, image.image, image.allocation);
        if (image.view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, image.view, nullptr);
        }
        if (image.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, image.sampler, nullptr);
        }
    }
}