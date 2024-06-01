#include <renderer/pathtraced_renderer.h>
#include <engine_context.h>

namespace core {

    PathTracedRenderer::PathTracedRenderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapchain, const std::vector<std::shared_ptr<DescriptorSet>>& globalDescSets) : 
        Renderer(device, graphicsQueue, presentQueue, swapchain), m_globalDescSets(globalDescSets) {
    
        CreateRenderPass();
        CreateFramebuffers();
        CreateDescriptorSets();
        CreatePipeline();
    }

    void PathTracedRenderer::CreateRenderPass() {
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

        VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
    }

    void PathTracedRenderer::CreateFramebuffers() {
        m_swapchainFramebuffers.resize(m_swapchain.imageViews.size());

        for (size_t i = 0; i < m_swapchain.imageViews.size(); i++) {
            std::array<VkImageView, 1> attachments = { m_swapchain.imageViews[i] };

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

    //***************************************************************************************//
    //                              Descriptor Sets & Pipelines                              //
    //***************************************************************************************//

    void PathTracedRenderer::CreateDescriptorSets() {
        m_rtDescSet = std::make_shared<DescriptorSet>();
        // Bind top-level acceleration structure descriptor.
        m_rtDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Bind render pass output image descriptor.
        m_rtDescSet->addDescriptor(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        // Bind object descriptions buffer descriptor.
        m_rtDescSet->addDescriptor(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create descriptor set.
        m_rtDescSet->create(EngineContext::GetInstance().getDevice(), MAX_FRAMES_IN_FLIGHT);
        m_rtDescSet->setName("RayTracing - RayTrace");

        m_postDescSet = std::make_shared<DescriptorSet>();
        // Bind render pass input image descriptor.
        m_postDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // Create descriptor set.
        m_postDescSet->create(EngineContext::GetInstance().getDevice(), MAX_FRAMES_IN_FLIGHT);
        m_postDescSet->setName("RayTracing - Post");
    }

    void PathTracedRenderer::CreatePipeline() {
        // Create list of descriptor set layouts for raytracing pipeline.
        std::vector<VkDescriptorSetLayout> rtLayouts;
        for (const auto& descSet : m_globalDescSets) rtLayouts.push_back(descSet->getSetLayout());
        rtLayouts.push_back(m_rtDescSet->getSetLayout());

        // Create list of descriptor set layouts for post pipeline.
        std::vector<VkDescriptorSetLayout> postLayouts{ m_postDescSet->getSetLayout() };

        // Create pipelines.
        m_rtPipeline = std::make_unique<RayTracingPipeline>(m_device, rtLayouts);
        m_postPipeline = std::make_unique<PostPipeline>(m_device, "postShader", postLayouts, m_renderPass, m_swapchain.extent);
    }

    void PathTracedRenderer::InitDescriptorSets(Scene& scene) {
        // Initialize descriptors for every frames in flight.
        VkDescriptorImageInfo imageDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // Upload TLAS descriptor.
            VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
            tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            tlasDescInfo.accelerationStructureCount = 1;
            tlasDescInfo.pAccelerationStructures = &scene.getTLAS().handle;
            m_rtDescSet->writeAccelerationStructureKHR(i, 0, tlasDescInfo);

            // Upload ray-tracing render pass output image uniform.
            std::unique_ptr<Texture> outTexture = std::make_unique<Texture>(m_swapchain.extent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            EngineContext::GetInstance().transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            m_rtDescTextures.push_back(std::move(outTexture));

            imageDescInfos[i].sampler = m_rtDescTextures[i]->getSampler();
            imageDescInfos[i].imageView = m_rtDescTextures[i]->getImageView();
            imageDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            m_rtDescSet->writeImage(i, 1, imageDescInfos[i]);
            m_postDescSet->writeImage(i, 0, imageDescInfos[i]);

            // Upload objects description buffer.
            VkDescriptorBufferInfo objDescInfo{};
            objDescInfo.buffer = scene.getObjDescriptions().buffer;
            objDescInfo.offset = 0;
            objDescInfo.range = VK_WHOLE_SIZE;
            m_rtDescSet->writeBuffer(i, 2, objDescInfo);
        }
        // Update descriptor sets.
        m_rtDescSet->update();
        m_postDescSet->update();
    }

    void PathTracedRenderer::UpdateDescriptorSets(Scene& scene) {
        // TODO
    }

    //***************************************************************************************//
    //                                       Rendering                                       //
    //***************************************************************************************//

    void PathTracedRenderer::RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline->GetHandle());

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> descSets{ m_globalDescSets[0]->getHandle(currentFrame), m_globalDescSets[1]->getHandle(0), m_rtDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline->GetLayout(), 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);

        // Upload push constants.
        RayTracingPushConstant constant;
        constant.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
        vkCmdPushConstants(commandBuffer, m_rtPipeline->GetLayout(), constant.getShaderStageFlags(), 0, constant.getSize(), &constant);

        // Draw ray-traced.
        SBTWrapper sbt = m_rtPipeline->GetSBT();
        uint32_t width = m_swapchain.extent.width;
        uint32_t height = m_swapchain.extent.height;
        vkCmdTraceRaysKHR(commandBuffer, &sbt.rgenRegion, &sbt.missRegion, &sbt.hitRegion, &sbt.callRegion, width, height, 1);

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = m_swapchain.extent;
        VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postPipeline->GetHandle());

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
        std::vector<VkDescriptorSet> postDescSets{ m_postDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postPipeline->GetLayout(), 0, static_cast<uint32_t>(postDescSets.size()), postDescSets.data(), 0, nullptr);

        // Draw
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        // End command buffer.
        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }
}