#include <resource_allocator.h>
#include <engine_globals.h>
#include <engine_context.h>

namespace core {

    VkDevice ResourceAllocator::device;
	VkCommandPool ResourceAllocator::commandPool;
	VkQueue ResourceAllocator::transferQueue;
	VmaAllocator ResourceAllocator::allocator;

    VkDeviceAddress Buffer::getDeviceAddress() {
        return ResourceAllocator::getBufferDeviceAddress(this->buffer);
    }

    bool Buffer::isCreated() {
        return !(this->buffer == VK_NULL_HANDLE);
    }

    VkDeviceAddress AccelerationStructure::getDeviceAddress() {
        return ResourceAllocator::getBufferDeviceAddress(this->buffer);
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

    void ResourceAllocator::createBuffer(const VkDeviceSize& size, VkBuffer& buffer, VmaAllocation& allocation, VkBufferUsageFlags usage) {
        VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VmaAllocationInfo allocInfo;
        VK_CHECK(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo));
    }

    void ResourceAllocator::createBuffer(const VkDeviceSize& size, Buffer& buffer, VkBufferUsageFlags usage) {
        createBuffer(size, buffer.buffer, buffer.allocation, usage);
    }

    void ResourceAllocator::createBufferWithAlignment(const VkDeviceSize& size, const VkDeviceSize& minAlignment, VkBuffer& buffer, VmaAllocation& allocation, VkBufferUsageFlags usage) {
        VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VmaAllocationInfo allocInfo;
        VK_CHECK(vmaCreateBufferWithAlignment(allocator, &bufferCreateInfo, &allocCreateInfo, minAlignment, &buffer, &allocation, &allocInfo));
    }

    void ResourceAllocator::createBufferWithAlignment(const VkDeviceSize& size, const VkDeviceSize& minAlignment, Buffer& buffer, VkBufferUsageFlags usage) {
        createBufferWithAlignment(size, minAlignment, buffer.buffer, buffer.allocation, usage);
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

    VkDeviceAddress ResourceAllocator::getBufferDeviceAddress(const VkBuffer& buffer) {
        VkBufferDeviceAddressInfo addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.buffer = buffer;

        return vkGetBufferDeviceAddress(device, &addressInfo);
    }

    VkDeviceAddress ResourceAllocator::getBufferDeviceAddress(const Buffer& buffer) {
        return getBufferDeviceAddress(buffer.buffer);
    }

    void ResourceAllocator::destroyBuffer(const VkBuffer& buffer, const VmaAllocation& allocation) {
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

	void ResourceAllocator::destroyBuffer(const Buffer& buffer) {
        destroyBuffer(buffer.buffer, buffer.allocation);
    }

    //***************************************************************************************//
    //                                   Image Allocation                                    //
    //***************************************************************************************//

    void ResourceAllocator::createImage2D(const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, Image& image, const VkImageUsageFlags& usage) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.usage = usage;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        imageAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        VK_CHECK(vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &image.image, &image.allocation, nullptr));
    }

    void ResourceAllocator::createAndStageImage2D(const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, const void* data, Image& image, VkImageUsageFlags usage) {
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
        createAndStageImage2D(commandBuffer, extent, format, mipLevels, data, srcBuffer, image, usage);

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

    void ResourceAllocator::createAndStageImage2D(const VkCommandBuffer& commandBuffer, const VkExtent2D& extent, const VkFormat& format, const uint32_t mipLevels, const void* data, Buffer& srcBuffer, Image& dstImage, VkImageUsageFlags usage) {
        // Create source buffer.
        VkDeviceSize size = extent.width * extent.height * 4;
        createBuffer(size, srcBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        
        // Map data to source buffer.
        mapDataToBuffer(srcBuffer, size, data);

        // Create destination image.
        createImage2D(extent, format, mipLevels, dstImage, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage);

        // Transition image layout.
        EngineContext::transitionImageLayout(commandBuffer, dstImage.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy data from source buffer to destination image.
        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0; // TODO - copy region for each mip levels.
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = {0, 0, 0};
        copyRegion.imageExtent = {extent.width, extent.height, 1};
        vkCmdCopyBufferToImage(commandBuffer, srcBuffer.buffer, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    void ResourceAllocator::createImageView2D(VkImage& image, VkImageView& view, const VkFormat& format, const VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = format;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask = aspectFlags;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &view));
    }

    void ResourceAllocator::createSampler2D(const VkSamplerAddressMode& addressMode, const bool& enableAnisotropy, VkImage& image, VkSampler& sampler) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = enableAnisotropy;
        samplerInfo.maxAnisotropy = EngineContext::getPhysicalDeviceProperties().deviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
    }

    void ResourceAllocator::destroyImage(const VkImage& image, const VmaAllocation& allocation) {
        vmaDestroyImage(allocator, image, allocation);
    }

	void ResourceAllocator::destroyImage(const Image& image) {
        destroyImage(image.image, image.allocation);
    }

    void ResourceAllocator::destroyImageView(const VkImageView& view) {
        vkDestroyImageView(device, view, nullptr);
    }

    void ResourceAllocator::destroySampler(const VkSampler& sampler) {
        vkDestroySampler(device, sampler, nullptr);
    }

    //***************************************************************************************//
    //                          Acceleration Structure Allocation                            //
    //***************************************************************************************//

    void ResourceAllocator::createAccelerationStructure(const VkDeviceSize& size, AccelerationStructure& accelStruct, const VkAccelerationStructureTypeKHR& type) {
        createBuffer(size, accelStruct.buffer, accelStruct.allocation, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		
        VkAccelerationStructureCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.type = type;
		createInfo.size = size;
        createInfo.buffer = accelStruct.buffer;

		VK_CHECK(vkCreateAccelerationStructureKHR(device, &createInfo, nullptr, &accelStruct.handle));
    }

	void ResourceAllocator::destroyAccelerationStructure(const AccelerationStructure& accelStruct) {
        if (accelStruct.handle != VK_NULL_HANDLE) {
            destroyBuffer(accelStruct.buffer, accelStruct.allocation);
            vkDestroyAccelerationStructureKHR(device, accelStruct.handle, nullptr);
        }
    }
}