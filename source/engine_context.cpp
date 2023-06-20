#include <engine_globals.h>
#include <engine_context.h>
#include <rtime.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <renderer.h>

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
void load_extension_VK_EXT_debug_utils(VkInstance);
void load_extension_VK_EXT_debug_report(VkInstance);

namespace core {

    Window EngineContext::window;
    VkInstance EngineContext::instance;
    vk::SurfaceKHR EngineContext::surface;
    std::vector<const char*> EngineContext::instanceExtensions;
    std::vector<const char*> EngineContext::deviceExtensions;
    std::vector<const char*> EngineContext::layers;

    PhysicalDeviceProperties EngineContext::physicalDeviceProperties;
    vk::PhysicalDevice EngineContext::physicalDevice = VK_NULL_HANDLE;
    vk::Device EngineContext::device = VK_NULL_HANDLE;
    vk::Queue EngineContext::graphicsQueue = VK_NULL_HANDLE;
    vk::Queue EngineContext::presentQueue = VK_NULL_HANDLE;

    vk::CommandPool EngineContext::commandPool;

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
            ResourceAllocator::setup(instance, physicalDevice, device, queryQueueFamilies(physicalDevice).graphicsFamily.value());
            Debugger::setup(instance, device);
            createCommandPool();
            Renderer::setup(device, graphicsQueue, presentQueue);
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
        Renderer::cleanup();
        device.destroyCommandPool(commandPool);
        Debugger::cleanup();
        ResourceAllocator::cleanup();
        device.destroy();
        vkDestroySurfaceKHR(instance, surface, nullptr);
        window.cleanup();
        vkDestroyInstance(instance, nullptr);
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
        instanceExtensions.push_back("VK_EXT_debug_utils");
        if (Debugger::debugReportEnabled) {
            instanceExtensions.push_back("VK_EXT_debug_report");
        }
    }

    void EngineContext::setupValidationLayers() {
        // Use validation layers if this is a debug build
#if defined(_DEBUG)
        layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    }

    void EngineContext::createInstance() {
        // Application information.
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan C++ Windowed Program";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "LunarG SDK";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = ENGINE_GRAPHICS_API_VERSION;
        
        void* pValidationFeatures = nullptr;
        if (Debugger::debugReportEnabled) {
            // List of all knronos validation layer features to enable.
            VkValidationFeatureEnableEXT enabledValidationFeatures[1] = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
            // Validation features.
            VkValidationFeaturesEXT validationFeatures{};
            validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            validationFeatures.enabledValidationFeatureCount = 1;
            validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;
            // Set pValidationFeatures.
            pValidationFeatures = &validationFeatures;
        }

        // Instance create info.
        VkInstanceCreateInfo instInfo{};
        instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instInfo.pApplicationInfo = &appInfo;
        instInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instInfo.ppEnabledExtensionNames = instanceExtensions.data();
        instInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instInfo.ppEnabledLayerNames = layers.data();
        instInfo.pNext = pValidationFeatures;

        // Create the Vulkan instance.
        try {
            vkCreateInstance(&instInfo, nullptr, &instance);
        } catch (const std::exception& e) {
            throw std::runtime_error("Could not create a Vulkan instance: " + *e.what());
        }

        // load VK instance extensions.
        load_extension_VK_EXT_debug_utils(instance);
        if (Debugger::debugReportEnabled) {
            load_extension_VK_EXT_debug_report(instance);
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
        deviceExtensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME); // Required to access 16 bit storage data from shaders.
        if (Debugger::debugReportEnabled) {
            deviceExtensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME); // Required for the "GLSL_EXT_debug_printf" shader extension.
        }
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
        EngineContext::physicalDeviceProperties = {deviceProperties.properties, raytracingProperties, accelProperties};
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
        VkPhysicalDevice16BitStorageFeaturesKHR shader16bitFeature{};
        shader16bitFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
        shader16bitFeature.storageBuffer16BitAccess = VK_TRUE;

        VkPhysicalDeviceDescriptorIndexingFeatures descIndexFeature{};
        descIndexFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descIndexFeature.runtimeDescriptorArray = VK_TRUE;
        descIndexFeature.pNext = &shader16bitFeature;

        VkPhysicalDeviceTimelineSemaphoreFeatures timeSemFeature{};
        timeSemFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
        timeSemFeature.timelineSemaphore = VK_TRUE;
        timeSemFeature.pNext = &descIndexFeature;

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

    void EngineContext::exit() {
        window.quit();
    }
}