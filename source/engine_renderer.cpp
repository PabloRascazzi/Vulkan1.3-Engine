#include <engine_renderer.h>
#include <engine_context.h>
#include <SDL2/SDL_vulkan.h>

namespace core {

	void EngineRenderer::setup() {
		createSwapChain();
        createDescriptorSets();
	}

    void EngineRenderer::cleanup() {
        // Cleanup renderers.
        delete standardRenderer;
        delete raytracedRenderer;
        delete pathtracedRenderer;
        // Cleanup descriptor sets.
        delete globalDescSet;
        // Cleanup swapchain.
        swapchain.destroy(EngineContext::getDevice());
    }

	//***************************************************************************************//
	//                                       Swapchain                                       //
	//***************************************************************************************//

    VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
	VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D selectSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR capabilities);

	void EngineRenderer::createSwapChain() {
        QueueFamilyIndices indices = EngineContext::getQueueFamilyIndices();
        SwapChainSupport swapChainSupport = EngineContext::getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = selectSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = selectSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = selectSwapChainExtent((SDL_Window*)EngineContext::getWindow().getHandle(), swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        // Make sure the image count does not exceed the maximum. Special value of 0 means no maximum.
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = EngineContext::getSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (indices.graphicsFamily != indices.presentFamily) {
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
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

        VK_CHECK(vkCreateSwapchainKHR(EngineContext::getDevice(), &createInfo, nullptr, &swapchain.handle));
        swapchain.format = surfaceFormat.format;
        swapchain.extent = extent;

        // Fetch all swap chain images handles.
        VK_CHECK(vkGetSwapchainImagesKHR(EngineContext::getDevice(), swapchain.handle, &imageCount, nullptr));
        swapchain.images.resize(imageCount);
        swapchain.imageViews.resize(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(EngineContext::getDevice(), swapchain.handle, &imageCount, (VkImage*)swapchain.images.data()));

        // Create swap chain image views.
        for (size_t i = 0; i < swapchain.images.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapchain.images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapchain.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(EngineContext::getDevice(), &createInfo, nullptr, &swapchain.imageViews[i]));
        }
	}

    VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) {
        // Look for desired surface format from list of available formats.
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        // Return first available format if could not find desired one.
        return availableFormats[0];
    }

    VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> presentModes) {
        // Look for desired surface present mode from list of available present modes.
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        // Return first available present mode if could not find desired one.
        return presentModes[0];
    }

    VkExtent2D selectSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            SDL_Vulkan_GetDrawableSize(window, &width, &height);

            VkExtent2D extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    void EngineRenderer::updateSwapchain() {
        // TODO
    }

    //***************************************************************************************//
    //                                    Descriptor Sets                                    //
    //***************************************************************************************//

    void EngineRenderer::createDescriptorSets() {
        globalDescSet = new DescriptorSet();
        // Bind camera descriptor.
        globalDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Bind texture array descriptor.
        globalDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, unknown, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create descriptor set.
        globalDescSet->create(EngineContext::getDevice());
    }

    void EngineRenderer::updateDescriptorSets() {
        updateCameraDescriptor();
        // TODO
    }

    void EngineRenderer::updateCameraDescriptor() {
        struct CameraUniformBufferObject {
            glm::mat4 viewProj;    // view * projection
            glm::mat4 viewInverse; // inverse view matrix
            glm::mat4 projInverse; // inverse projection matrix
            glm::vec3 position; // camera's world position
        };

        CameraUniformBufferObject cameraUBO{};
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
        // TODO - cameraUBO.position = scene->getMainCamera().getWorldPosition();

        // Create this frame's descriptor buffer if not yet created.
        if (!cameraDescBuffers[currentSwapchainIndex].isCreated()) {
            ResourceAllocator::createBuffer(sizeof(CameraUniformBufferObject), cameraDescBuffers[currentSwapchainIndex], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        }
        // Map camera uniform buffer data to this frame's descriptor buffer.
        ResourceAllocator::mapDataToBuffer(cameraDescBuffers[currentSwapchainIndex], sizeof(CameraUniformBufferObject), &cameraUBO);

        // Write camera uniform buffer to current frame's descriptor set.
        VkDescriptorBufferInfo camDescInfo{};
        camDescInfo.buffer = cameraDescBuffers[currentSwapchainIndex].buffer;
        camDescInfo.offset = 0;
        camDescInfo.range = sizeof(CameraUniformBufferObject);
        globalDescSet->writeBuffer(0, camDescInfo);
    }

    void EngineRenderer::updateTextureArrayDescriptor() {
        // TODO - check if scene has added or removed any textures.
        // TODO - if so, write updated texture array to descriptor set.
        // TODO - if not, do nothing.
    }

    //***************************************************************************************//
    //                                       Renderers                                       //
    //***************************************************************************************//

    void EngineRenderer::createRenderers() {
        // TODO
    }

    void EngineRenderer::createPipelines() {
        // TODO
    }

    void EngineRenderer::render() {
        updateDescriptorSets();
        // TODO - call appropriate renderer's render function.
    }
}