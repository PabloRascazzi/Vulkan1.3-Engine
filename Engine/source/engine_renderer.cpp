#include <engine_renderer.h>
#include <engine_context.h>
#include <SDL2/SDL_vulkan.h>
#include <renderer/standard_renderer.h>
#include <renderer/pathtraced_renderer.h>
#include <renderer/gaussian_renderer.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>

namespace core {

    // Swapchain for the main window surface.
    Swapchain EngineRenderer::swapchain;
    uint32_t EngineRenderer::currentSwapchainIndex;

    // Current scene to render.
    Scene* EngineRenderer::scene;

    // Global Descriptor Sets.
    DescriptorSet* EngineRenderer::cameraDescSet;
    DescriptorSet* EngineRenderer::texturesDescSet;

    // Global Descriptor buffer.
    Buffer EngineRenderer::cameraDescBuffer;
    uint32_t EngineRenderer::cameraDescBufferAlignment;

    // Instances for all the renderers.
    Renderer* EngineRenderer::standardRenderer;
    Renderer* EngineRenderer::raytracedRenderer;
    Renderer* EngineRenderer::pathtracedRenderer;
    Renderer* EngineRenderer::gaussianRenderer;

	void EngineRenderer::setup(Scene* scene) {
        EngineRenderer::scene = scene;
		createSwapChain();
        createDescriptorSets();
        createRenderers();
        initDescriptorSets();
	}

    void EngineRenderer::cleanup() {
        // Cleanup renderers.
        delete static_cast<StandardRenderer*>(standardRenderer);
        delete static_cast<PathTracedRenderer*>(pathtracedRenderer);
        delete static_cast<GaussianRenderer*>(gaussianRenderer);
        // Cleanup descriptor buffers
        ResourceAllocator::destroyBuffer(cameraDescBuffer);
        // Cleanup descriptor sets.
        delete cameraDescSet;
        delete texturesDescSet;
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
        // TODO - recreate swapchain
    }

    //***************************************************************************************//
    //                                    Descriptor Sets                                    //
    //***************************************************************************************//

    struct CameraDescriptorBuffer {
        glm::mat4 viewProj;    // view * projection
        glm::mat4 viewInverse; // inverse view matrix
        glm::mat4 projInverse; // inverse projection matrix
        glm::vec3 position;    // camera's world position
    };

    void EngineRenderer::createDescriptorSets() {
        // Allocate camera descriptor set.
        cameraDescSet = new DescriptorSet();
        // Bind camera descriptor.
        cameraDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Create descriptor set.
        cameraDescSet->create(EngineContext::getDevice(), MAX_FRAMES_IN_FLIGHT);
        cameraDescSet->setName("Camera");

        // Allocate texture descriptor set.
        texturesDescSet = new DescriptorSet();
        // Bind texture array descriptor.
        texturesDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
        // Create descriptor set.
        texturesDescSet->create(EngineContext::getDevice());
        texturesDescSet->setName("Textures");
    }

    void EngineRenderer::initDescriptorSets() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
        // TODO - cameraUBO.position = scene->getMainCamera().getWorldPosition();

        // Allocate and map data to camera desc buffer.
        cameraDescBufferAlignment = static_cast<uint32_t>(alignUp(EngineContext::getPhysicalDeviceProperties().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(CameraDescriptorBuffer)));
        CameraDescriptorBuffer cameraUBOs[MAX_FRAMES_IN_FLIGHT]{ cameraUBO };
        ResourceAllocator::createBuffer(cameraDescBufferAlignment * MAX_FRAMES_IN_FLIGHT, cameraDescBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        ResourceAllocator::mapDataToBuffer(cameraDescBuffer, cameraDescBufferAlignment * MAX_FRAMES_IN_FLIGHT, &cameraUBO);

        // Upload camera descriptor to all descriptor sets.
        VkDescriptorBufferInfo camDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            camDescInfos[i].buffer = cameraDescBuffer.buffer;
            camDescInfos[i].offset = cameraDescBufferAlignment * i;
            camDescInfos[i].range = sizeof(CameraDescriptorBuffer);
            cameraDescSet->writeBuffer(i, 0, camDescInfos[i]);
        }

        // Upload texture sampler descriptors.
        VkDescriptorImageInfo* globalTextureDescInfos = new VkDescriptorImageInfo[scene->getTextures().size()];
        for (uint32_t i = 0; i < scene->getTextures().size(); i++) {
            globalTextureDescInfos[i].sampler = scene->getTextures()[i]->getSampler();
            globalTextureDescInfos[i].imageView = scene->getTextures()[i]->getImageView();
            globalTextureDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texturesDescSet->writeImage(0, 0, globalTextureDescInfos[i], i);
        }

        // Update global desc sets.
        cameraDescSet->update();
        texturesDescSet->update();
        delete[] globalTextureDescInfos;
    }

    void EngineRenderer::updateDescriptorSets() {
        updateCameraDescriptor();
        updateTextureArrayDescriptor();
    }

    void EngineRenderer::updateCameraDescriptor() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
        // TODO - cameraUBO.position = scene->getMainCamera().getWorldPosition();

        // Map new camera descriptor buffer data to current frame's camera descriptor buffer.
        ResourceAllocator::mapDataToBuffer(cameraDescBuffer, sizeof(CameraDescriptorBuffer), &cameraUBO, cameraDescBufferAlignment * currentSwapchainIndex);
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
        standardRenderer = new StandardRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue(), EngineRenderer::getSwapchain(), std::vector<DescriptorSet*>{ cameraDescSet, texturesDescSet });
        pathtracedRenderer = new PathTracedRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue(), EngineRenderer::getSwapchain(), std::vector<DescriptorSet*>{ cameraDescSet, texturesDescSet });
        gaussianRenderer = new GaussianRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue(), EngineRenderer::getSwapchain(), std::vector<DescriptorSet*>{ cameraDescSet });
    }

    void EngineRenderer::render(const uint8_t rendermode) {
        // Update all descriptors.
        updateDescriptorSets();

        // Select renderer and render scene.
        switch (rendermode) {
            case 0: static_cast<StandardRenderer*>(standardRenderer)->render(currentSwapchainIndex, *scene); break;
            case 1: static_cast<PathTracedRenderer*>(pathtracedRenderer)->render(currentSwapchainIndex, *scene); break;
            case 2: static_cast<GaussianRenderer*>(gaussianRenderer)->render(currentSwapchainIndex, *scene); break;
            default: break;
        }

        // Set next swapchain frame index.
        currentSwapchainIndex = (currentSwapchainIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}