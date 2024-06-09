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

	EngineRenderer::EngineRenderer(const std::shared_ptr<Scene>& scene) : m_scene(scene), m_currentSwapchainIndex(0), m_context(EngineContext::GetInstance()) {
		CreateSwapChain();
        CreateDescriptorSets();
        CreateRenderers();
        InitDescriptorSets();
	}

    EngineRenderer::~EngineRenderer() {
        // Cleanup descriptor buffers
        ResourceAllocator::destroyBuffer(m_cameraDescBuffer);
        // Cleanup swapchain.
        m_swapchain.Destroy(m_context.getDevice());
    }

	//***************************************************************************************//
	//                                       Swapchain                                       //
	//***************************************************************************************//

    VkSurfaceFormatKHR SelectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats);
	VkPresentModeKHR SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D SelectSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR capabilities);

	void EngineRenderer::CreateSwapChain() {
        QueueFamilyIndices indices = m_context.getQueueFamilyIndices();
        SwapChainSupport swapChainSupport = m_context.getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = SelectSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = SelectSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = SelectSwapChainExtent((SDL_Window*)m_context.getWindow().getHandle(), swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        // Make sure the image count does not exceed the maximum. Special value of 0 means no maximum.
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_context.getSurface();
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

        VK_CHECK(vkCreateSwapchainKHR(m_context.getDevice(), &createInfo, nullptr, &m_swapchain.handle));
        m_swapchain.format = surfaceFormat.format;
        m_swapchain.extent = extent;

        // Fetch all swap chain images handles.
        VK_CHECK(vkGetSwapchainImagesKHR(m_context.getDevice(), m_swapchain.handle, &imageCount, nullptr));
        m_swapchain.images.resize(imageCount);
        m_swapchain.imageViews.resize(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(m_context.getDevice(), m_swapchain.handle, &imageCount, (VkImage*)m_swapchain.images.data()));

        // Create swap chain image views.
        for (size_t i = 0; i < m_swapchain.images.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapchain.images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapchain.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(m_context.getDevice(), &createInfo, nullptr, &m_swapchain.imageViews[i]));
        }
	}

    VkSurfaceFormatKHR SelectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) {
        // Look for desired surface format from list of available formats.
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        // Return first available format if could not find desired one.
        return availableFormats[0];
    }

    VkPresentModeKHR SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR> presentModes) {
        // Look for desired surface present mode from list of available present modes.
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        // Return first available present mode if could not find desired one.
        return presentModes[0];
    }

    VkExtent2D SelectSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR capabilities) {
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

    void EngineRenderer::UpdateSwapchain() {
        // TODO - recreate swapchain
    }

    //***************************************************************************************//
    //                                    Descriptor Sets                                    //
    //***************************************************************************************//

    struct CameraDescriptorBuffer {
        glm::mat4 view;        // View matrix
        glm::mat4 viewInverse; // Inverse view matrix
        glm::mat4 proj;        // Projection matrix
        glm::mat4 projInverse; // Inverse projection matrix
        glm::mat4 viewProj;    // View matrix * projection matrix
        glm::vec2 focal;       // Focal length = Resolution / (2.0 * tan(FOV * 0.5))
        glm::vec2 tanFOV;      // Tangent of half FOV = tan(FOV * 0.5)
        glm::vec3 position;    // World position
    };

    void EngineRenderer::CreateDescriptorSets() {
        // Allocate camera descriptor set.
        m_cameraDescSet = std::make_shared<DescriptorSet>();
        // Bind camera descriptor.
        m_cameraDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT);
        // Create descriptor set.
        m_cameraDescSet->create(m_context.getDevice(), MAX_FRAMES_IN_FLIGHT);
        m_cameraDescSet->setName("Camera");

        // Allocate texture descriptor set.
        m_texturesDescSet = std::make_shared<DescriptorSet>();
        // Bind texture array descriptor.
        m_texturesDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
        // Create descriptor set.
        m_texturesDescSet->create(m_context.getDevice());
        m_texturesDescSet->setName("Textures");
    }

    void EngineRenderer::InitDescriptorSets() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.view = m_scene->getMainCamera().GetViewMatrix();
        cameraUBO.viewInverse = m_scene->getMainCamera().GetViewInverseMatrix();
        cameraUBO.proj = m_scene->getMainCamera().GetProjectionMatrix();
        cameraUBO.projInverse = m_scene->getMainCamera().GetProjectionInverseMatrix();
        cameraUBO.viewProj = m_scene->getMainCamera().GetProjectionMatrix() * m_scene->getMainCamera().GetViewMatrix();
        cameraUBO.tanFOV = glm::tan(m_scene->getMainCamera().GetFOV() * 0.5f);
        cameraUBO.focal = glm::vec2(m_swapchain.extent.width, m_swapchain.extent.height) / (2.0f * cameraUBO.tanFOV);
        cameraUBO.position = m_scene->getMainCamera().GetWorldPosition();

        // Allocate and map data to camera desc buffer.
        m_cameraDescBufferAlignment = alignUp(sizeof(CameraDescriptorBuffer), m_context.getPhysicalDeviceProperties().deviceProperties.limits.minUniformBufferOffsetAlignment);
        CameraDescriptorBuffer cameraUBOs[MAX_FRAMES_IN_FLIGHT]{ cameraUBO };
        ResourceAllocator::createBuffer(m_cameraDescBufferAlignment * MAX_FRAMES_IN_FLIGHT, m_cameraDescBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        ResourceAllocator::mapDataToBuffer(m_cameraDescBuffer, m_cameraDescBufferAlignment * MAX_FRAMES_IN_FLIGHT, &cameraUBO);

        // Upload camera descriptor to all descriptor sets.
        VkDescriptorBufferInfo camDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            camDescInfos[i].buffer = m_cameraDescBuffer.buffer;
            camDescInfos[i].offset = m_cameraDescBufferAlignment * i;
            camDescInfos[i].range = sizeof(CameraDescriptorBuffer);
            m_cameraDescSet->writeBuffer(i, 0, camDescInfos[i]);
        }

        // Upload texture sampler descriptors.
        VkDescriptorImageInfo* globalTextureDescInfos = new VkDescriptorImageInfo[m_scene->getTextures().size()];
        for (uint32_t i = 0; i < m_scene->getTextures().size(); i++) {
            globalTextureDescInfos[i].sampler = m_scene->getTextures()[i]->getSampler();
            globalTextureDescInfos[i].imageView = m_scene->getTextures()[i]->getImageView();
            globalTextureDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_texturesDescSet->writeImage(0, 0, globalTextureDescInfos[i], i);
        }

        // Update global desc sets.
        m_cameraDescSet->update();
        m_texturesDescSet->update();
        delete[] globalTextureDescInfos;
    }

    void EngineRenderer::UpdateDescriptorSets() {
        UpdateCameraDescriptor();
        UpdateTextureArrayDescriptor();
    }

    void EngineRenderer::UpdateCameraDescriptor() {
        // Create camera descriptor buffer.
        CameraDescriptorBuffer cameraUBO{};
        cameraUBO.view = m_scene->getMainCamera().GetViewMatrix();
        cameraUBO.viewInverse = m_scene->getMainCamera().GetViewInverseMatrix();
        cameraUBO.proj = m_scene->getMainCamera().GetProjectionMatrix();
        cameraUBO.projInverse = m_scene->getMainCamera().GetProjectionInverseMatrix();
        cameraUBO.viewProj = m_scene->getMainCamera().GetProjectionMatrix() * m_scene->getMainCamera().GetViewMatrix();
        cameraUBO.tanFOV = glm::tan(m_scene->getMainCamera().GetFOV() * 0.5f);
        cameraUBO.focal = cameraUBO.focal = glm::vec2(m_swapchain.extent.width, m_swapchain.extent.height) / (2.0f * cameraUBO.tanFOV);
        cameraUBO.position = m_scene->getMainCamera().GetWorldPosition();

        // Map new camera descriptor buffer data to current frame's camera descriptor buffer.
        ResourceAllocator::mapDataToBuffer(m_cameraDescBuffer, sizeof(CameraDescriptorBuffer), &cameraUBO, m_cameraDescBufferAlignment * m_currentSwapchainIndex);
    }

    void EngineRenderer::UpdateTextureArrayDescriptor() {
        // TODO - check if scene has added or removed any textures.
        // TODO - if so, write updated texture array to descriptor set.
        // TODO - if not, do nothing.
    }

    //***************************************************************************************//
    //                                       Renderers                                       //
    //***************************************************************************************//

    void EngineRenderer::CreateRenderers() {
        m_standardRenderer = std::make_unique<StandardRenderer>(m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getPresentQueue(), m_swapchain, std::vector<std::shared_ptr<DescriptorSet>>{ m_cameraDescSet, m_texturesDescSet });
        m_pathtracedRenderer = std::make_unique<PathTracedRenderer>(m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getPresentQueue(), m_swapchain, std::vector<std::shared_ptr<DescriptorSet>>{ m_cameraDescSet, m_texturesDescSet });
        m_gaussianRenderer = std::make_unique<GaussianRenderer>(m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getPresentQueue(), m_swapchain, std::vector<std::shared_ptr<DescriptorSet>>{ m_cameraDescSet });
    }

    void EngineRenderer::Render(const uint8_t rendermode) {
        // Update all descriptors.
        UpdateDescriptorSets();

        // Select renderer and render scene.
        switch (rendermode) {
            case 0: m_standardRenderer->Render(m_currentSwapchainIndex, *m_scene); break;
            case 1: m_pathtracedRenderer->Render(m_currentSwapchainIndex, *m_scene); break;
            case 2: m_gaussianRenderer->Render(m_currentSwapchainIndex, *m_scene); break;
            default: break;
        }

        // Set next swapchain frame index.
        m_currentSwapchainIndex = (m_currentSwapchainIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}