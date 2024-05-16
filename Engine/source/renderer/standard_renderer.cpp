#include <renderer/standard_renderer.h>
#include <engine_context.h>

namespace core {

    StandardRenderer::StandardRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapchain, const std::vector<DescriptorSet*> globalDescSets) : 
        Renderer(device, graphicsQueue, presentQueue, swapchain), m_globalDescSets(globalDescSets), m_depthImageFormat(VK_FORMAT_D32_SFLOAT) {
    
        CreateRenderPass();
        CreateFramebuffers();
        CreateDescriptorSets();
        CreatePipeline();
    }

    StandardRenderer::~StandardRenderer() {
        delete m_pipeline;
        ResourceAllocator::destroyImageView(m_depthImageView);
        ResourceAllocator::destroyImage(m_depthImage);
    }

    void StandardRenderer::CreateFramebuffers() {
        // Create depth image.
        ResourceAllocator::createImage2D(m_swapchain.extent, m_depthImageFormat, 1, m_depthImage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        ResourceAllocator::createImageView2D(m_depthImage.image, m_depthImageView, m_depthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        EngineContext::transitionImageLayout(m_depthImage.image, m_depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        
        // Create framebuffers.
        m_swapchainFramebuffers.resize(m_swapchain.imageViews.size());

        for (size_t i = 0; i < m_swapchain.imageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = { m_swapchain.imageViews[i], m_depthImageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapchain.extent.width;
            framebufferInfo.height = m_swapchain.extent.height;
            framebufferInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]));
        }
    }

    void StandardRenderer::CreateRenderPass() {
        VkAttachmentDescription colorAttachmentDesc{};
        colorAttachmentDesc.format = m_swapchain.format;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachmentDesc{};
        depthAttachmentDesc.format = m_depthImageFormat;
        depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachmentDesc, depthAttachmentDesc };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
    }

    //***************************************************************************************//
    //                              Descriptor Sets & Pipelines                              //
    //***************************************************************************************//

    void StandardRenderer::CreateDescriptorSets() {
        // TODO
    }

    void StandardRenderer::CreatePipeline() {
        std::vector<VkDescriptorSetLayout> layouts;
        for (const auto& descSet : m_globalDescSets) layouts.push_back(descSet->getSetLayout());
        m_pipeline = new StandardPipeline(m_device, "shader", layouts, m_renderPass, m_swapchain.extent);
    }

    void StandardRenderer::InitDescriptorSets(Scene& scene) {
        // TODO
    }

    void StandardRenderer::UpdateDescriptorSets(Scene& scene) {
        // TODO
    }

    //***************************************************************************************//
    //                                      Renderering                                      //
    //***************************************************************************************//

    void StandardRenderer::RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Render pass clear values
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = m_swapchain.extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetHandle());

        // Set Viewport and Scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchain.extent.width);
        viewport.height = static_cast<float>(m_swapchain.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0,0 };
        scissor.extent = m_swapchain.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> descSets = { m_globalDescSets[0]->getHandle(currentFrame), m_globalDescSets[1]->getHandle(0) };
        if (descSets.size() > 0) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetLayout(), 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);
        }

        for (const auto& object : scene.getObjects()) {
            // Bind vertex buffer
            VkBuffer vertexBuffers[] = { object.mesh->getVertexBuffer().buffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            for (uint32_t i = 0; i < object.mesh->getSubmeshCount() && i < object.materials.size(); i++) {
                // Upload push constants
                StandardPushConstant constant;
                constant.world = object.transform;
                constant.view = scene.getMainCamera().getViewMatrix();
                constant.proj = scene.getMainCamera().getProjectionMatrix();
                constant.materialAddress = object.materials.at(i)->getBuffer().getDeviceAddress();
                vkCmdPushConstants(commandBuffer, m_pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, constant.getSize(), &constant);

                // Bind index buffer
                vkCmdBindIndexBuffer(commandBuffer, object.mesh->getSubmesh(i).getIndexBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);

                // Draw
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->getSubmesh(i).getIndexCount()), 1, 0, 0, 0);
            }
        }

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }
}
