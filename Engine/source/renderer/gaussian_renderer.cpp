#include <renderer/gaussian_renderer.h>
#include <engine_context.h>

namespace core {

	GaussianRenderer::GaussianRenderer(VkDevice device, VkQueue computeQueue, VkQueue presentQueue, Swapchain& swapChain, const std::vector<DescriptorSet*>& globalDescSets) :
		Renderer(device, computeQueue, presentQueue), swapChain(swapChain), globalDescSets(globalDescSets), firstRender(true) {
    
        createRenderPass();
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorSets();
        createPipeline(device);
    }

	GaussianRenderer::~GaussianRenderer() {
        // Cleanup pipelines.
        delete gsPipeline;
        delete postPipeline;
        // Cleanup descriptor buffers.
        for (auto& texture : gsDescTextures)
            delete texture;
        // Cleanup descriptor sets.
        delete gsDescSet;
        delete postDescSet;
        // Cleanup sync objects.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        vkDestroyRenderPass(device, renderPass, nullptr);
	}

    void GaussianRenderer::createRenderPass() {
        VkAttachmentDescription colorAttachmentDesc{};
        colorAttachmentDesc.format = swapChain.format;
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
        subpass.pDepthStencilAttachment = nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 1> attachments = { colorAttachmentDesc };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    }

    void GaussianRenderer::createFramebuffers() {
        swapChainFramebuffers.resize(swapChain.imageViews.size());

        for (size_t i = 0; i < swapChain.imageViews.size(); i++) {
            std::array<VkImageView, 1> attachments = { swapChain.imageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChain.extent.width;
            framebufferInfo.height = swapChain.extent.height;
            framebufferInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]));
        }
    }

    void GaussianRenderer::createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        EngineContext::createCommandBuffer(commandBuffers.data(), MAX_FRAMES_IN_FLIGHT);
    }

    void GaussianRenderer::createSyncObjects() {
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

    //***************************************************************************************//
    //                              Descriptor Sets & Pipelines                              //
    //***************************************************************************************//

    void GaussianRenderer::createDescriptorSets() {
        gsDescSet = new DescriptorSet();
        // Bind render pass output image descriptor.
        gsDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);
        // Bind preprocessed gaussian geometry buffer descriptor.
        gsDescSet->addDescriptor(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
        // Create descriptor set.
        gsDescSet->create(EngineContext::getDevice(), MAX_FRAMES_IN_FLIGHT);
        gsDescSet->setName("Gaussian Splatting - Rasterize");

        postDescSet = new DescriptorSet();
        // Bind render pass input image descriptor.
        postDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // Create descriptor set.
        postDescSet->create(EngineContext::getDevice(), MAX_FRAMES_IN_FLIGHT);
        postDescSet->setName("Gaussian Splatting - Post");
    }

    void GaussianRenderer::initDescriptorSets(Scene& scene) {
        // Initialize descriptors for every frames in flight.
        VkDescriptorImageInfo imageDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // Upload ray-tracing render pass output image uniform.
            Texture* outTexture = new Texture(swapChain.extent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            EngineContext::transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            gsDescTextures.push_back(outTexture);

            imageDescInfos[i].sampler = gsDescTextures[i]->getSampler();
            imageDescInfos[i].imageView = gsDescTextures[i]->getImageView();
            imageDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            gsDescSet->writeImage(i, 0, imageDescInfos[i]);
            postDescSet->writeImage(i, 0, imageDescInfos[i]);

            // Upload preprocessed gaussian geometry buffer.
            // TODO - VkDescriptorBufferInfo geomDescInfo{};
            // TODO - geomDescInfo.buffer = scene.getObjDescriptions().buffer;
            // TODO - geomDescInfo.offset = 0;
            // TODO - geomDescInfo.range = VK_WHOLE_SIZE;
            // TODO - gsDescSet->writeBuffer(i, 2, geomDescInfo);
        }
        // Update descriptor sets.
        gsDescSet->update();
        postDescSet->update();
    }

    void GaussianRenderer::updateDescriptorSets() {
        // TODO
    }

    void GaussianRenderer::createPipeline(VkDevice device) {
        // Create list of descriptor set layouts for gaussian rasterization compute pipeline.
        std::vector<VkDescriptorSetLayout> gsLayouts;
        for (const auto& descSet : globalDescSets) gsLayouts.push_back(descSet->getSetLayout());
        gsLayouts.push_back(this->gsDescSet->getSetLayout());

        // Create list of descriptor set layouts for post pipeline.
        std::vector<VkDescriptorSetLayout> postLayouts{ this->postDescSet->getSetLayout() };

        // Create pipelines.
        gsPipeline = new ComputePipeline(device, "gaussian_rasterize", gsLayouts);
        postPipeline = new PostPipeline(device, "postShader", postLayouts, this->renderPass, this->swapChain.extent);
    }

    //***************************************************************************************//
    //                                       Rendering                                       //
    //***************************************************************************************//

    void GaussianRenderer::recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->gsPipeline->getHandle());

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> descSets{ this->globalDescSets[0]->getHandle(currentFrame), this->gsDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->gsPipeline->getLayout(), 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);

        // Compute Gaussian Rasterization
        uint32_t width = swapChain.extent.width;
        uint32_t height = swapChain.extent.height;
        vkCmdDispatch(commandBuffer, width, height, 1);

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = swapChain.extent;
        VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->postPipeline->getHandle());

        // Set Viewport and Scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.extent.width);
        viewport.height = static_cast<float>(swapChain.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0,0 };
        scissor.extent = swapChain.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> postDescSets{ this->postDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->postPipeline->getLayout(), 0, static_cast<uint32_t>(postDescSets.size()), postDescSets.data(), 0, nullptr);

        // Draw
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        // End command buffer.
        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void GaussianRenderer::render(const uint32_t currentFrame, Scene& scene) {
        // Initialize descriptor sets on first render.
        if (firstRender) {
            initDescriptorSets(scene);
            firstRender = false;
        }

        // Update descriptor sets.
        updateDescriptorSets();

        // Wait until previous frame has finished.
        VK_CHECK(vkWaitForFences(device, 1, (VkFence*)&inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(device, 1, (VkFence*)&inFlightFences[currentFrame]));

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        VK_CHECK(vkAcquireNextImageKHR(device, swapChain.handle, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex));

        // Reset command buffer.
        VK_CHECK(vkResetCommandBuffer(commandBuffers[currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        // Record command buffer.
        recordCommandBuffer((VkCommandBuffer&)commandBuffers[currentFrame], currentFrame, imageIndex, scene);

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

        VK_CHECK_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw gaussians command buffer.");

        // Presentation.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains[] = { swapChain.handle };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));
    }
}