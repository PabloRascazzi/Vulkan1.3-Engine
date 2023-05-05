#include <renderer.h>
#include <engine_globals.h>
#include <engine_context.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>

namespace core {

    VkDevice Renderer::device;
    VkQueue Renderer::graphicsQueue;
    VkQueue Renderer::presentQueue;

	VkRenderPass Renderer::renderPass = VK_NULL_HANDLE;
    VkSwapchainKHR Renderer::swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> Renderer::swapChainImages;
    std::vector<VkImageView> Renderer::swapChainImageViews;
    VkFormat Renderer::swapChainImageFormat;
    VkExtent2D Renderer::swapChainExtent;
    std::vector<VkFramebuffer> Renderer::swapChainFramebuffers;

    std::vector<VkCommandBuffer> Renderer::commandBuffers;
	std::vector<VkSemaphore> Renderer::imageAvailableSemaphores;
	std::vector<VkSemaphore> Renderer::renderFinishedSemaphores;
	std::vector<VkFence> Renderer::inFlightFences;

    uint32_t Renderer::currentFrame = 0;

	void Renderer::setup(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue) {
        Renderer::device = device;
        Renderer::graphicsQueue = graphicsQueue;
        Renderer::presentQueue = presentQueue;
		createSwapChain();
        createImageViews();
        createRenderPass();
        createFramebuffers();
        createFrameCommandBuffers();
        createSyncObjects();
	}

	void Renderer::cleanup() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        vkDestroyRenderPass(device, renderPass, nullptr);
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

    void Renderer::createSwapChain() {
        QueueFamilyIndices indices = EngineContext::getQueueFamilyIndices();
        SwapChainSupport swapChainSupport = EngineContext::getSwapChainSupport();

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
        createInfo.surface = EngineContext::getSurface();
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

        VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));
        Renderer::swapChainImageFormat = surfaceFormat.format;
        Renderer::swapChainExtent = extent;

        // Fetch all swap chain images handles.
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr));
        swapChainImages.resize(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, (VkImage*)swapChainImages.data()));
    }

    VkSurfaceFormatKHR Renderer::selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) {
        // Look for desired surface format from list of available formats.
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        // Return first available format if could not find desired one.
        return availableFormats[0];
    }

    VkPresentModeKHR Renderer::selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> presentModes) {
        // Look for desired surface present mode from list of available present modes.
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        // Return first available present mode if could not find desired one.
        return presentModes[0];
    }

    VkExtent2D Renderer::selectSwapChainExtent(const VkSurfaceCapabilitiesKHR capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            SDL_Vulkan_GetDrawableSize((SDL_Window*)EngineContext::getWindow().getHandle(), &width, &height);

            VkExtent2D extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    void Renderer::createRenderPass() {
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

        VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    }

    void Renderer::createImageViews() {
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

            VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]));
        }
    }

    void Renderer::createFramebuffers() {
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

            VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]));
        }
    }

    void Renderer::createFrameCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        EngineContext::createCommandBuffer(commandBuffers.data(), MAX_FRAMES_IN_FLIGHT);
    }

    void Renderer::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start fence in signaled state.

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
        }
    }

    void Renderer::recordRasterizeCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex, Pipeline& pipeline, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

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

            // Upload push constants
            vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, constant.getSize(), &constant);

            // Draw
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->getIndexCount()), 1, 0, 0, 0);
        }

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void Renderer::rasterize(Pipeline& pipeline, Scene& scene) {
        // Wait until previous frame has finished.
        VK_CHECK(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(device, 1, &inFlightFences[currentFrame]));

        // Set descriptor set currentFrame.
        DescriptorSet::setCurrentFrame(currentFrame);

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        VK_CHECK(vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex));

        // Reset command buffer.
        VK_CHECK(vkResetCommandBuffer(commandBuffers[currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        // Record command buffer.
        recordRasterizeCommandBuffer(commandBuffers[currentFrame], imageIndex, pipeline, scene);

        // Submit command buffer.
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw rasterized command buffer.");

        // Presentation.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));

        // Set next frame index.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::recordRaytraceCommandBuffer(const VkCommandBuffer& commandBuffer, Pipeline& rtPipeline, Pipeline& postPipeline, std::vector<Image>& outImages, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

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
        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void Renderer::raytrace(Pipeline& rtPipeline, Pipeline& postPipeline, Scene& scene, std::vector<Image>& outImages) {
        // Wait until previous frame has finished.
        VK_CHECK(vkWaitForFences(device, 1, (VkFence*)&inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(device, 1, (VkFence*)&inFlightFences[currentFrame]));
        
        // Set descriptor set currentFrame.
        DescriptorSet::setCurrentFrame(currentFrame);

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        VK_CHECK(vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex));

        // Reset command buffer.
        VK_CHECK(vkResetCommandBuffer(commandBuffers[currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        // Record command buffer.
        recordRaytraceCommandBuffer((VkCommandBuffer&)commandBuffers[currentFrame], rtPipeline, postPipeline, outImages, imageIndex);

        // Submit command buffer.
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw ray-traced command buffer.");

        // Presentation.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));

        // Set next frame index.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}