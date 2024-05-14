#include <renderer/pathtraced_renderer.h>
#include <engine_context.h>

namespace core {

    PathTracedRenderer::PathTracedRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapChain, std::vector<DescriptorSet*> globalDescSets) : Renderer(device, graphicsQueue, presentQueue), swapChain(swapChain) {
        this->firstRender = true;
        this->globalDescSets = globalDescSets;
        createRenderPass();
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorSets();
        createPipeline(device);
    }

    PathTracedRenderer::~PathTracedRenderer() {
        this->cleanup();
    }

    void PathTracedRenderer::cleanup() {
        // Cleanup pipelines.
        rtPipeline->cleanup();
        postPipeline->cleanup();
        // Cleanup descriptor buffers.
        for (auto& texture : rtDescTextures)
            delete texture;
        // Cleanup descriptor sets.
        delete rtDescSet;
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

    void PathTracedRenderer::createRenderPass() {
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

    void PathTracedRenderer::createFramebuffers() {
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

    void PathTracedRenderer::createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        EngineContext::createCommandBuffer(commandBuffers.data(), MAX_FRAMES_IN_FLIGHT);
    }

    void PathTracedRenderer::createSyncObjects() {
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

    void PathTracedRenderer::createDescriptorSets() {
        rtDescSet = new DescriptorSet();
        // Bind top-level acceleration structure descriptor.
        rtDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Bind render pass output image descriptor.
        rtDescSet->addDescriptor(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Bind object descriptions buffer descriptor.
        rtDescSet->addDescriptor(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create descriptor set.
        rtDescSet->create(EngineContext::getDevice(), MAX_FRAMES_IN_FLIGHT);
        rtDescSet->setName("RayTracing");

        postDescSet = new DescriptorSet();
        // Bind render pass input image descriptor.
        postDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // Create descriptor set.
        postDescSet->create(EngineContext::getDevice(), MAX_FRAMES_IN_FLIGHT);
        postDescSet->setName("Post");
    }

    void PathTracedRenderer::initDescriptorSets(Scene& scene) {
        // Initialize descriptors for every frames in flight.
        VkDescriptorImageInfo imageDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // Upload TLAS descriptor.
            VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
            tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            tlasDescInfo.accelerationStructureCount = 1;
            tlasDescInfo.pAccelerationStructures = &scene.getTLAS().handle;
            rtDescSet->writeAccelerationStructureKHR(i, 0, tlasDescInfo);

            // Upload ray-tracing render pass output image uniform.
            Texture* outTexture = new Texture(swapChain.extent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            EngineContext::transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            rtDescTextures.push_back(outTexture);

            imageDescInfos[i].sampler = rtDescTextures[i]->getSampler();
            imageDescInfos[i].imageView = rtDescTextures[i]->getImageView();
            imageDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            rtDescSet->writeImage(i, 1, imageDescInfos[i]);
            postDescSet->writeImage(i, 0, imageDescInfos[i]);

            // Upload objects description buffer.
            VkDescriptorBufferInfo objDescInfo{};
            objDescInfo.buffer = scene.getObjDescriptions().buffer;
            objDescInfo.offset = 0;
            objDescInfo.range = VK_WHOLE_SIZE;
            rtDescSet->writeBuffer(i, 2, objDescInfo);
        }
        // Update descriptor sets.
        rtDescSet->update();
        postDescSet->update();
    }

    void PathTracedRenderer::updateDescriptorSets() {
        // TODO
    }

    void PathTracedRenderer::createPipeline(VkDevice device) {
        // Create list of descriptor set layouts for raytracing pipeline.
        std::vector<VkDescriptorSetLayout> rtLayouts;
        for (const auto& descSet : globalDescSets) rtLayouts.push_back(descSet->getSetLayout());
        rtLayouts.push_back(this->rtDescSet->getSetLayout());

        // Create list of descriptor set layouts for post pipeline.
        std::vector<VkDescriptorSetLayout> postLayouts{ this->postDescSet->getSetLayout() };

        // Create pipelines.
        rtPipeline = new RayTracingPipeline(device, rtLayouts);
        postPipeline = new PostPipeline(device, "postShader", postLayouts, this->renderPass, this->swapChain.extent);
    }

    //***************************************************************************************//
    //                                      Renderering                                      //
    //***************************************************************************************//

    void PathTracedRenderer::recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->rtPipeline->getHandle());

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> descSets{ this->globalDescSets[0]->getHandle(currentFrame), this->globalDescSets[1]->getHandle(0), this->rtDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->rtPipeline->getLayout(), 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);

        // Upload push constants.
        RayTracingPushConstant constant;
        constant.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
        vkCmdPushConstants(commandBuffer, this->rtPipeline->getLayout(), constant.getShaderStageFlags(), 0, constant.getSize(), &constant);

        // Draw ray-traced.
        SBTWrapper sbt = static_cast<RayTracingPipeline*>(this->rtPipeline)->getSBT();
        uint32_t width = swapChain.extent.width;
        uint32_t height = swapChain.extent.height;
        vkCmdTraceRaysKHR(commandBuffer, &sbt.rgenRegion, &sbt.missRegion, &sbt.hitRegion, &sbt.callRegion, width, height, 1);

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

    void PathTracedRenderer::render(const uint32_t currentFrame, Scene& scene) {
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

        VK_CHECK_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw ray-traced command buffer.");

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