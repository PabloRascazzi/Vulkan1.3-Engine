#include <engine_context.h>
#include <time.h>

#include <vulkan/vulkan.hpp>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include <iostream>
#include <vector>

// Removes minwindef.h max() definition which overlaps with limits.h max().
#ifdef max 
#undef max
#endif

namespace core {

    Window EngineContext::window;
    vk::Instance EngineContext::instance;
    vk::SurfaceKHR EngineContext::surface;
    std::vector<const char*> EngineContext::instanceExtensions;
    std::vector<const char*> EngineContext::deviceExtensions;
    std::vector<const char*> EngineContext::layers;

    vk::PhysicalDevice EngineContext::physicalDevice = VK_NULL_HANDLE;
    vk::Device EngineContext::device = VK_NULL_HANDLE;
    vk::Queue EngineContext::graphicsQueue = VK_NULL_HANDLE;
    vk::Queue EngineContext::presentQueue = VK_NULL_HANDLE;

    vk::CommandPool EngineContext::commandPool;
    VmaAllocator EngineContext::allocator;

    vk::RenderPass EngineContext::renderPass = VK_NULL_HANDLE;
    vk::SwapchainKHR EngineContext::swapChain = VK_NULL_HANDLE;
    std::vector<vk::Image> EngineContext::swapChainImages;
    std::vector<vk::ImageView> EngineContext::swapChainImageViews;
    vk::Format EngineContext::swapChainImageFormat;
    vk::Extent2D EngineContext::swapChainExtent;

    void EngineContext::setup() {
        try {
            window.setup();
            setupInstanceExtensions();
            setupValidationLayers();
            createInstance();
            createSurface();
            setupDeviceExtensions();
            selectPhysicalDevice();
            createLogicalDevice();
            createCommandPool();
            createAllocator();
            createSwapChain();
            createImageViews();
            createRenderPass();
        }
        catch (const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            std::exit(1);
        }
        Time::setup();
    }

    bool EngineContext::update() {
        // Time code
        Time::update();
#if defined(_DEBUG)
        Time::fpsCounter();
#endif
        return window.update();
    }

    void EngineContext::cleanup() {
        // Destroy all vulkan objects
        device.destroyRenderPass(renderPass);
        for (auto imageView : swapChainImageViews) {
            device.destroyImageView(imageView);
        }
        device.destroySwapchainKHR(swapChain);
        vmaDestroyAllocator(allocator);
        device.destroyCommandPool(commandPool);
        device.destroy();
        instance.destroySurfaceKHR(surface);
        window.cleanup();
        instance.destroy();
    }

    void EngineContext::setupInstanceExtensions() {
        // Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
        unsigned extension_count;
        if (!SDL_Vulkan_GetInstanceExtensions((SDL_Window*)window.getHandle(), &extension_count, NULL)) {
            throw std::runtime_error("Could not get the number of required instance extensions from SDL.");
        }

        instanceExtensions.resize(extension_count);
        if (!SDL_Vulkan_GetInstanceExtensions((SDL_Window*)window.getHandle(), &extension_count, instanceExtensions.data())) {
            throw std::runtime_error("Could not get the names of required instance extensions from SDL.");
        }
    }

    void EngineContext::setupValidationLayers() {
        // Use validation layers if this is a debug build
#if defined(_DEBUG)
        layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    }

    void EngineContext::createInstance() {
        // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
        // program, which can be useful for layers and tools to provide more debug information.
        vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setPApplicationName("Vulkan C++ Windowed Program")
            .setApplicationVersion(1)
            .setPEngineName("LunarG SDK")
            .setEngineVersion(1)
            .setApiVersion(ENGINE_GRAPHICS_API_VERSION);

        // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
        // are needed.
        vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
            .setFlags(vk::InstanceCreateFlags())
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()))
            .setPpEnabledExtensionNames(instanceExtensions.data())
            .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
            .setPpEnabledLayerNames(layers.data());

        // Create the Vulkan instance.
        try {
            instance = vk::createInstance(instInfo);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Could not create a Vulkan instance: " + *e.what());
        }
    }

    void EngineContext::createSurface() {
        // Create a Vulkan surface for rendering
        VkSurfaceKHR c_surface;
        if (!SDL_Vulkan_CreateSurface((SDL_Window*)window.getHandle(), static_cast<VkInstance>(instance), &c_surface)) {
            throw std::runtime_error("Could not create a Vulkan surface.");
        }
        surface = c_surface;
    }

    void EngineContext::setupDeviceExtensions() {
        // Add swap chain extension
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    void EngineContext::selectPhysicalDevice() {
        // Fetch the amount of physical devices that support Vulkan.
        Uint32 deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Could not find GPUs with Vulkan support.");
        }

        // Select first physical device that is a discrete GPU with a geometry shader. 
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader && isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        // Check that a physical device was successfully selected.
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Could not find GPU with suitable properties.");
        }
        EngineContext::physicalDevice = physicalDevice;
    }

    void EngineContext::createLogicalDevice() {
        QueueFamilyIndices indices = queryQueueFamilies(physicalDevice);

        // Generate create info for each queue families.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t index : indices.toVector()) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = index;
            queueCreateInfo.queueCount = 1;
            // Number between 0.0 and 1.0. Determines queue priority if there are many queues per queue family.
            float queuePriority = 1.0;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        // TODO - fetch device features wanted.

        // Generate create info for logical device.
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
#if defined(_DEBUG)
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        deviceCreateInfo.ppEnabledLayerNames = layers.data();
#else
        deviceCreateInfo.enabledLayerCount = 0;
#endif

        // Create logical device using device create info.
        VkDevice device;
        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Could not create logical device.");
        }
        EngineContext::device = device;

        // Cache graphics queue handle.
        VkQueue graphicsQueue;
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        EngineContext::graphicsQueue = graphicsQueue;

        // Cache present queue handle.
        VkQueue presentQueue;
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
        EngineContext::presentQueue = presentQueue;
    }

    bool EngineContext::isDeviceSuitable(VkPhysicalDevice device) {
        return queryQueueFamilies(device).isComplete()
            && checkDeviceExtensionSupport(device)
            && querySwapChainSupport(device).isAdequate();
    }

    QueueFamilyIndices EngineContext::queryQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            else {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport) {
                    indices.presentFamily = i;
                }
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    bool EngineContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupport EngineContext::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupport support;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            support.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, support.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            support.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, support.presentModes.data());
        }

        return support;
    }

    void EngineContext::createAllocator() {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = ENGINE_GRAPHICS_API_VERSION;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.pVulkanFunctions = nullptr; // optional

        if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
            throw std::runtime_error("Could not create memory allocator.");
        }
    }

    void EngineContext::createSwapChain() {
        QueueFamilyIndices indices = queryQueueFamilies(physicalDevice);
        SwapChainSupport swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = selectSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = selectSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = selectSwapChainExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        // Make sure the image count does not exceed the maximum. Special value of 0 means no maximum.
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (indices.graphicsFamily != indices.presentFamily) {
            uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain;
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("Could not create swap chain.");
        }
        EngineContext::swapChain = swapChain;
        EngineContext::swapChainImageFormat = (vk::Format)(surfaceFormat.format);
        EngineContext::swapChainExtent = extent;

        // Fetch all swap chain images handles.
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, (VkImage*)swapChainImages.data());
    }

    void EngineContext::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = (VkFormat)swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::runtime_error("Could not create image views.");
            }
            swapChainImageViews[i] = imageView;
        }
    }

    void EngineContext::createRenderPass() {
	    VkAttachmentDescription colorAttachmentDesc{};
	    colorAttachmentDesc.format = (VkFormat)swapChainImageFormat;
	    colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	    colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	    colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	    colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	    VkAttachmentReference colorAttachmentRef{};
	    colorAttachmentRef.attachment = 0;
	    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    VkSubpassDescription subpass{};
	    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	    subpass.colorAttachmentCount = 1;
	    subpass.pColorAttachments = &colorAttachmentRef;

	    VkSubpassDependency dependency{};
	    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	    dependency.dstSubpass = 0;
	    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	    dependency.srcAccessMask = 0;
	    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	    VkRenderPassCreateInfo renderPassInfo{};
	    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	    renderPassInfo.attachmentCount = 1;
	    renderPassInfo.pAttachments = &colorAttachmentDesc;
	    renderPassInfo.subpassCount = 1;
	    renderPassInfo.pSubpasses = &subpass;
	    renderPassInfo.dependencyCount = 1;
	    renderPassInfo.pDependencies = &dependency;

	    VkRenderPass renderPass;
	    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		    throw std::runtime_error("Could not create render pass.");
	    }
	    EngineContext::renderPass = renderPass;
    }

    VkSurfaceFormatKHR EngineContext::selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) {
        // Look for desired surface format from list of available formats.
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        // Return first available format if could not find desired one.
        return availableFormats[0];
    }

    VkPresentModeKHR EngineContext::selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> presentModes) {
        // Look for desired surface present mode from list of available present modes.
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        // Return first available present mode if could not find desired one.
        return presentModes[0];
    }

    VkExtent2D EngineContext::selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            SDL_Vulkan_GetDrawableSize((SDL_Window*)window.getHandle(), &width, &height);

            VkExtent2D extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    void EngineContext::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = queryQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        VkCommandPool commandPool;
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Could not create command pool.");
        }
        EngineContext::commandPool = commandPool;
    }

    void EngineContext::exit() {
        window.quit();
    }

    //***************************************************************************************//
    //                                   Memory Allocator                                    //
    //***************************************************************************************//

    void EngineContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& alloc) {
        VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VmaAllocationInfo allocInfo;
        if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &alloc, &allocInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer.");
        }
    }

    void EngineContext::mapBufferData(VmaAllocation& alloc, size_t size, void* data) {
        void* location;
        vmaMapMemory(allocator, alloc, &location);
        memcpy(location, data, size);
        vmaUnmapMemory(allocator, alloc);
    }

    void EngineContext::copyBufferData(VkBuffer& srcBuffer, VkBuffer& dstBuffer, size_t size) {
        // Create new command buffer.
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

        // Record command buffer.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        // Destroy command buffer
        vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
    }

    void EngineContext::destroyBuffer(VkBuffer& buffer, VmaAllocation& alloc) {
        vmaDestroyBuffer(allocator, buffer, alloc);
    }
}