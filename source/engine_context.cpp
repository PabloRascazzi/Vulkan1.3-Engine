#include <engine_context.h>
#include <time.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>

#include <vulkan/vulkan.hpp>
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <resource_allocator.h>

#include <iostream>
#include <vector>
#include <string>

// Removes minwindef.h max() definition which overlaps with limits.h max().
#ifdef max 
#undef max
#endif

void load_extension_VK_KHR_acceleration_structure(VkDevice);
void load_extension_VK_KHR_deferred_host_operations(VkDevice);
void load_extension_VK_KHR_ray_tracing_pipeline(VkDevice);

#define VK_CHECK_MSG(func, msg) if(func != VK_SUCCESS) { throw std::runtime_error(msg); }
#define VK_CHECK(func) VK_CHECK_MSG(func, "Vulkan error detected at line " + std::to_string(__LINE__) + " .");

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

    vk::PhysicalDeviceProperties EngineContext::deviceProperties;
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR EngineContext::rtProperties;
	vk::PhysicalDeviceAccelerationStructurePropertiesKHR EngineContext::asProperties;

    vk::CommandPool EngineContext::commandPool;
    VmaAllocator EngineContext::allocator;

    vk::RenderPass EngineContext::renderPass = VK_NULL_HANDLE;
    vk::SwapchainKHR EngineContext::swapChain = VK_NULL_HANDLE;
    std::vector<vk::Image> EngineContext::swapChainImages;
    std::vector<vk::ImageView> EngineContext::swapChainImageViews;
    vk::Format EngineContext::swapChainImageFormat;
    vk::Extent2D EngineContext::swapChainExtent;
    std::vector<vk::Framebuffer> EngineContext::swapChainFramebuffers;

    std::vector<vk::CommandBuffer> EngineContext::commandBuffers;
	std::vector<vk::Semaphore> EngineContext::imageAvailableSemaphores;
	std::vector<vk::Semaphore> EngineContext::renderFinishedSemaphores;
	std::vector<vk::Fence> EngineContext::inFlightFences;

    uint32_t EngineContext::currentFrame = 0;

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
            createFramebuffers();
            createFrameCommandBuffers();
            createSyncObjects();
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
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device.destroySemaphore(imageAvailableSemaphores[i]);
            device.destroySemaphore(renderFinishedSemaphores[i]);
            device.destroyFence(inFlightFences[i]);
        }
        for (auto framebuffer : swapChainFramebuffers) {
            device.destroyFramebuffer(framebuffer);
        }
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
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Required to build swapchains.
        deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME); // Required to build acceleration structures.
        deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME); // Required for ray tracing pipeline.
        deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME); // Required to build and use ray tracing pipeline.
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

        // Fetch all properties.
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelProperties{};
        accelProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        accelProperties.pNext = nullptr;

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingProperties{};
        raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        raytracingProperties.pNext = &accelProperties;

        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties.pNext = &raytracingProperties;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
        EngineContext::deviceProperties = deviceProperties.properties;
        EngineContext::rtProperties = raytracingProperties;
        EngineContext::asProperties = accelProperties;
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

        // Setup physical device features to enable.
        VkPhysicalDeviceTimelineSemaphoreFeatures timeSemFeature{};
        timeSemFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
        timeSemFeature.timelineSemaphore = VK_TRUE;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{};
        accelFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelFeature.accelerationStructure = VK_TRUE;
        accelFeature.pNext = &timeSemFeature;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingFeature{};
        raytracingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        raytracingFeature.rayTracingPipeline = VK_TRUE;
        raytracingFeature.pNext = &accelFeature;

        VkPhysicalDeviceBufferDeviceAddressFeatures addressFeature{};
        addressFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        addressFeature.bufferDeviceAddress = VK_TRUE;
        addressFeature.pNext = &raytracingFeature;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt64 = VK_TRUE;

        VkPhysicalDeviceFeatures2 allDeviceFeatures{};
        allDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        allDeviceFeatures.features = deviceFeatures;
        allDeviceFeatures.pNext = &addressFeature;

        // Generate create info for logical device.
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = nullptr; // Must have device features ptr if pNext is nullptr else nullptr.
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pNext = &allDeviceFeatures;
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

        // load VK device extensions.
        load_extension_VK_KHR_acceleration_structure(device);
        load_extension_VK_KHR_deferred_host_operations(device);
        load_extension_VK_KHR_ray_tracing_pipeline(device);

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
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
            throw std::runtime_error("Could not create memory allocator.");
        }

        ResourceAllocator::setup(device, allocator, commandPool, graphicsQueue);
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

    void EngineContext::createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {swapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            VkFramebuffer framebuffer;
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                throw std::runtime_error("Could not create framebuffer.");
            }
            swapChainFramebuffers[i] = framebuffer;
        }
    }

    void EngineContext::createCommandBuffer(VkCommandBuffer* buffer, uint32_t amount) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = amount;

        if (vkAllocateCommandBuffers(device, &allocInfo, buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers.");
        }
    }

    void EngineContext::createFrameCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        createCommandBuffer((VkCommandBuffer*)commandBuffers.data(), MAX_FRAMES_IN_FLIGHT);
    }

    void EngineContext::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start fence in signaled state.

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, (VkSemaphore*)&imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, (VkSemaphore*)&renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, (VkFence*)&inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Could not create sync objects.");
            }
        }
    }

    void EngineContext::recordRasterizeCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Pipeline& pipeline, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Could not begin recording command buffer.");
        }

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());

        // Set Viewport and Scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0,0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        for (const auto& object : scene.getObjects()) {
            // Bind vertex buffer
            VkBuffer vertexBuffers[] = {object.mesh->getVertexBuffer().buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            // Bind index buffer
            vkCmdBindIndexBuffer(commandBuffer, object.mesh->getIndexBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);

            StandardPushConstant constant;
            constant.world = object.transform;
            constant.view = scene.getMainCamera().getViewMatrix();
            constant.proj = scene.getMainCamera().getProjectionMatrix();
            // TEST - static auto startTime = std::chrono::high_resolution_clock::now();
            // TEST - auto currentTime = std::chrono::high_resolution_clock::now();
            // TEST - float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            // TEST - constant.world = glm::rotate(object.transform, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            // Upload push constants
            vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, constant.getSize(), &constant);

            // Draw
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->getIndexCount()), 1, 0, 0, 0);
        }

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer.");
        }
    }

    void EngineContext::rasterize(Pipeline& pipeline, Scene& scene) {
        // Wait until previous frame has finished.
        device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        device.resetFences(1, &inFlightFences[currentFrame]);

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // Record command buffer.
        commandBuffers[currentFrame].reset();
        recordRasterizeCommandBuffer((VkCommandBuffer&)commandBuffers[currentFrame], imageIndex, pipeline, scene);

        // Submit command buffer.
        vk::SubmitInfo submitInfo{};
        vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (graphicsQueue.submit(1, (vk::SubmitInfo*)&submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to submit draw rasterized command buffer.");
        }

        // Presentation.
        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        vk::SwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        presentQueue.presentKHR(&presentInfo);

        // Set next frame index.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void EngineContext::transitionImageLayout(const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer;
        createCommandBuffer(&commandBuffer, 1);

        // Create memory transfer semaphore.
        VkSemaphoreTypeCreateInfo timelineSemaphoreInfo{};
        timelineSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineSemaphoreInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineSemaphoreInfo.initialValue = 0;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &timelineSemaphoreInfo;
        
        VkSemaphore imageTransitionComplete;
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageTransitionComplete));

        // Record command buffer.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        
        transitionImageLayout(commandBuffer, image, oldLayout, newLayout);

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
        submitInfo.pSignalSemaphores = &imageTransitionComplete;
        submitInfo.pNext = &timelineInfo;

        VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

        // Wait for memory transfer to complete.
        const uint64_t waitValue = 1;
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &imageTransitionComplete;
        waitInfo.pValues = &waitValue;
        VK_CHECK(vkWaitSemaphores(device, &waitInfo, UINT64_MAX));

        // Destroy command buffer and semaphore.
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroySemaphore(device, imageTransitionComplete, nullptr);
    }

    void EngineContext::transitionImageLayout(const VkCommandBuffer& commandBuffer, const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::runtime_error("Unsupported image layout transition.");
        }

        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void EngineContext::recordRaytraceCommandBuffer(const VkCommandBuffer& commandBuffer, Pipeline& rtPipeline, Pipeline& postPipeline, std::vector<Image>& outImages, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Could not begin recording command buffer.");
        }

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline.getHandle());
        
        // Bind descriptor sets.
        std::vector<VkDescriptorSet> descSets = rtPipeline.getDescriptorSetHandles();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline.getLayout(), 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);
        
        // Upload push constants.
        RayTracingPushConstant constant;
        constant.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
        vkCmdPushConstants(commandBuffer, rtPipeline.getLayout(), constant.getShaderStageFlags(), 0, constant.getSize(), &constant);

        // Draw ray-traced.
        SBTWrapper sbt = ((RayTracingPipeline*)&rtPipeline)->getSBT();
        uint32_t width = swapChainExtent.width;
        uint32_t height = swapChainExtent.height;
        vkCmdTraceRaysKHR(commandBuffer, &sbt.rgenRegion, &sbt.missRegion, &sbt.hitRegion, &sbt.callRegion, width, height, 1);

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.getHandle());

        // Set Viewport and Scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0,0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> postDescSets = postPipeline.getDescriptorSetHandles();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.getLayout(), 0, static_cast<uint32_t>(postDescSets.size()), postDescSets.data(), 0, nullptr);

        // Draw
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        // End command buffer.
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer.");
        }
    }

    void EngineContext::raytrace(Pipeline& rtPipeline, Pipeline& postPipeline, Scene& scene, std::vector<Image>& outImages) {
        // Wait until previous frame has finished.
        VK_CHECK(vkWaitForFences(device, 1, (VkFence*)&inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(device, 1, (VkFence*)&inFlightFences[currentFrame]));
        
        // Set descriptor set currentFrame.
        DescriptorSet::setCurrentFrame(currentFrame);

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        VK_CHECK(vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex));

        // Record command buffer.
        commandBuffers[currentFrame].reset();
        recordRaytraceCommandBuffer((VkCommandBuffer&)commandBuffers[currentFrame], rtPipeline, postPipeline, outImages, imageIndex);

        // Submit command buffer.
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = (VkCommandBuffer*)&commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw ray-traced command buffer.");

        // Presentation.
        VkSwapchainKHR swapChains[] = { swapChain };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));

        // Set next frame index.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void EngineContext::exit() {
        window.quit();
    }
}