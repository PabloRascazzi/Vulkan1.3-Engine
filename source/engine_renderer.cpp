#include <engine_renderer.h>
#include <engine_context.h>
#include <SDL2/SDL_vulkan.h>
#include <renderer/standard_renderer.h>
#include <renderer/pathtraced_renderer.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>

namespace core {

    // Swapchain for the main window surface.
    Swapchain EngineRenderer::swapchain;
    uint32_t EngineRenderer::currentSwapchainIndex;

    // Current scene to render.
    Scene* EngineRenderer::scene;

    // Descriptor Sets.
    DescriptorSet* EngineRenderer::globalDescSet;
    DescriptorSet* EngineRenderer::rtDescSet;
    DescriptorSet* EngineRenderer::postDescSet;

    // Descriptor buffers.
    std::vector<Buffer> EngineRenderer::cameraDescBuffers;
    std::vector<Texture*> EngineRenderer::rtDescTextures;

    // Instances for all the renderers.
    Renderer* EngineRenderer::standardRenderer;
    Renderer* EngineRenderer::raytracedRenderer;
    Renderer* EngineRenderer::pathtracedRenderer;

    // Instances for all the pipelines.
    Pipeline* EngineRenderer::standardPipeline;
    Pipeline* EngineRenderer::pathtracedPipeline;
    Pipeline* EngineRenderer::postPipeline;

	void EngineRenderer::setup(Scene* scene) {
        EngineRenderer::scene = scene;
		createSwapChain();
        createDescriptorSets();
        createRenderers();
        createPipelines();
        initDescriptorSets();
	}

    void EngineRenderer::cleanup() {
        // Cleanup renderers.
        standardRenderer->cleanup();
        pathtracedRenderer->cleanup();
        // Cleanup pipelines;
        standardPipeline->cleanup();
        pathtracedPipeline->cleanup();
        postPipeline->cleanup();
        // Cleanup descriptor buffers
        for (auto& buffer : cameraDescBuffers)
            ResourceAllocator::destroyBuffer(buffer);
        for (auto& texture : rtDescTextures)
            delete texture;
        // Cleanup descriptor sets.
        delete globalDescSet;
        delete rtDescSet;
        delete postDescSet;
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

    struct CameraDescriptorBuffer {
        glm::mat4 viewProj;    // view * projection
        glm::mat4 viewInverse; // inverse view matrix
        glm::mat4 projInverse; // inverse projection matrix
        glm::vec3 position;    // camera's world position
    };

    void EngineRenderer::createDescriptorSets() {
        globalDescSet = new DescriptorSet();
        // Bind camera descriptor.
        globalDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Bind texture array descriptor.
        globalDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(scene->getTextures().size()), VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create descriptor set.
        globalDescSet->create(EngineContext::getDevice());

        rtDescSet = new DescriptorSet();
        // Bind top-level acceleration structure descriptor.
        rtDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Bind render pass output image descriptor.
        rtDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Bind object descriptions buffer descriptor.
        rtDescSet->addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create descriptor set.
        rtDescSet->create(EngineContext::getDevice());

        postDescSet = new DescriptorSet();
        // Bind render pass input image descriptor.
        postDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // Create descriptor set.
        postDescSet->create(EngineContext::getDevice());
    }

    void EngineRenderer::initDescriptorSets() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
        // TODO - cameraUBO.position = scene->getMainCamera().getWorldPosition();

        // Fill DescriptorSets and create out images for ray-tracing render pass.
        cameraDescBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            DescriptorSet::setCurrentFrame(i);
    
            // Upload TLAS uniform.
            VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
            tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            tlasDescInfo.accelerationStructureCount = 1;
            tlasDescInfo.pAccelerationStructures = &scene->getTLAS().handle;
            rtDescSet->writeAccelerationStructureKHR(0, tlasDescInfo);
    
            // Upload ray-tracing render pass output image uniform.
            Texture* outTexture = new Texture(swapchain.extent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            EngineContext::transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            rtDescTextures.push_back(outTexture);
    
            VkDescriptorImageInfo rtImageDescInfo{};
            rtImageDescInfo.sampler = {};
            rtImageDescInfo.imageView = rtDescTextures[i]->getImageView();
            rtImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            rtDescSet->writeImage(1, rtImageDescInfo);
            
            VkDescriptorImageInfo postImageDescInfo{};
            postImageDescInfo.sampler = rtDescTextures[i]->getSampler();
            postImageDescInfo.imageView = rtDescTextures[i]->getImageView();
            postImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            postDescSet->writeImage(0, postImageDescInfo);
    
            // Upload objects description buffer.
            VkDescriptorBufferInfo objDescInfo{};
            objDescInfo.buffer = scene->getObjDescriptions().buffer;
            objDescInfo.offset = 0;
            objDescInfo.range = VK_WHOLE_SIZE;
            rtDescSet->writeBuffer(2, objDescInfo);

            // Upload camera matrices uniform.
            ResourceAllocator::createBuffer(sizeof(CameraDescriptorBuffer), cameraDescBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            ResourceAllocator::mapDataToBuffer(cameraDescBuffers[i], sizeof(CameraDescriptorBuffer), &cameraUBO);
    
            VkDescriptorBufferInfo camDescInfo{};
            camDescInfo.buffer = cameraDescBuffers[i].buffer;
            camDescInfo.offset = 0;
            camDescInfo.range = sizeof(CameraDescriptorBuffer);
            globalDescSet->writeBuffer(0, camDescInfo);
        }
        DescriptorSet::setCurrentFrame(0);
        rtDescSet->update();
        postDescSet->update();
        globalDescSet->update();

        // Upload Texture array descriptor.
        updateTextureArrayDescriptor();
    }

    void EngineRenderer::updateDescriptorSets() {
        updateCameraDescriptor();
        // TODO
    }

    void EngineRenderer::updateCameraDescriptor() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
        // TODO - cameraUBO.position = scene->getMainCamera().getWorldPosition();

        // Map new camera descriptor buffer data to current frame's camera descriptor buffer.
        ResourceAllocator::mapDataToBuffer(cameraDescBuffers[currentSwapchainIndex], sizeof(CameraDescriptorBuffer), &cameraUBO);

        // Write camera descriptor buffer to current frame's global descriptor set.
        VkDescriptorBufferInfo camDescInfo{};
        camDescInfo.buffer = cameraDescBuffers[currentSwapchainIndex].buffer;
        camDescInfo.offset = 0;
        camDescInfo.range = sizeof(CameraDescriptorBuffer);
        globalDescSet->writeBuffer(0, camDescInfo);
    }

    void EngineRenderer::updateTextureArrayDescriptor() {
        // TODO - check if scene has added or removed any textures.
        // TODO - if so, write updated texture array to descriptor set.
        // TODO - if not, do nothing.

        VkDescriptorImageInfo* globalTextureDescInfos = new VkDescriptorImageInfo[scene->getTextures().size() * MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            DescriptorSet::setCurrentFrame(i);

            // Upload texture samplers.
            for (uint32_t texIndex = 0; texIndex < scene->getTextures().size(); texIndex++) {
                size_t texDescIndex = texIndex + (scene->getTextures().size() * i);
                globalTextureDescInfos[texDescIndex].sampler = scene->getTextures()[texIndex]->getSampler();
                globalTextureDescInfos[texDescIndex].imageView = scene->getTextures()[texIndex]->getImageView();
                globalTextureDescInfos[texDescIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                globalDescSet->writeImage(1, globalTextureDescInfos[texDescIndex], texIndex);
            }
        }
        DescriptorSet::setCurrentFrame(0);
        globalDescSet->update();
        delete[] globalTextureDescInfos;
    }

    //***************************************************************************************//
    //                                       Renderers                                       //
    //***************************************************************************************//

    void EngineRenderer::createRenderers() {
        standardRenderer = new StandardRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue(), EngineRenderer::getSwapchain());
        pathtracedRenderer = new PathTracedRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue(), EngineRenderer::getSwapchain());
    }

    // TODO - move pipelines to Renderer classes.
    void EngineRenderer::createPipelines() {
        standardPipeline = new StandardPipeline(EngineContext::getDevice(), "shader", std::vector<DescriptorSet*>{ globalDescSet }, static_cast<StandardRenderer*>(standardRenderer)->getRenderPass(), EngineRenderer::getSwapchain().extent);
        pathtracedPipeline = new RayTracingPipeline(EngineContext::getDevice(), std::vector<DescriptorSet*>{ rtDescSet, globalDescSet });
        postPipeline = new PostPipeline(EngineContext::getDevice(), "postShader", std::vector<DescriptorSet*>{ postDescSet }, static_cast<PathTracedRenderer*>(pathtracedRenderer)->getRenderPass(), EngineRenderer::getSwapchain().extent);
    }

    void EngineRenderer::render(const bool raytrace) {
        // Update all descriptors.
        updateDescriptorSets();

        // Select renderer and render scene.
        if (raytrace) {
            static_cast<PathTracedRenderer*>(pathtracedRenderer)->render(currentSwapchainIndex, (Pipeline&)*pathtracedPipeline, (Pipeline&)*postPipeline, *scene);
        } else {
            static_cast<StandardRenderer*>(standardRenderer)->render(currentSwapchainIndex, (Pipeline&)*standardPipeline, *scene);
        }

        // Set next swapchain frame index.
        currentSwapchainIndex = (currentSwapchainIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}