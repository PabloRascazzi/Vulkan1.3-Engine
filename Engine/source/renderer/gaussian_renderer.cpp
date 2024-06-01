#include <renderer/gaussian_renderer.h>
#include <engine_context.h>
#include <debugger.h>
#include <../resource/shaders/GLSL/gaussian_common.h>

namespace core {

    const uint32_t num_gaussians = 0; // TODO

	GaussianRenderer::GaussianRenderer(VkDevice device, VkQueue computeQueue, VkQueue presentQueue, Swapchain& swapchain, const std::vector<std::shared_ptr<DescriptorSet>>& globalDescSets) :
		Renderer(device, computeQueue, presentQueue, swapchain), m_globalDescSets(globalDescSets) {
    
        CreateRenderPass();
        CreateFramebuffers();
        CreateDescriptorSets();
        CreatePipeline();
    }

	GaussianRenderer::~GaussianRenderer() {
        // Cleanup intermediary buffers.
        ResourceAllocator::destroyBuffer(m_geomBuffer);
        ResourceAllocator::destroyBuffer(m_keysBuffer);
	}

    void GaussianRenderer::CreateRenderPass() {
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

    void GaussianRenderer::CreateFramebuffers() {
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

    void GaussianRenderer::CreateDescriptorSets() {
        m_gsDescSet = std::make_shared<DescriptorSet>();
        // Bind render pass output image descriptor.
        m_gsDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);
        // Create descriptor set.
        m_gsDescSet->create(EngineContext::GetInstance().getDevice(), MAX_FRAMES_IN_FLIGHT);
        m_gsDescSet->setName("Gaussian Splatting - Rasterize");

        m_postDescSet = std::make_shared<DescriptorSet>();
        // Bind render pass input image descriptor.
        m_postDescSet->addDescriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // Create descriptor set.
        m_postDescSet->create(EngineContext::GetInstance().getDevice(), MAX_FRAMES_IN_FLIGHT);
        m_postDescSet->setName("Gaussian Splatting - Post");
    }

    void GaussianRenderer::CreatePipeline() {
        // Create list of descriptor set layouts for gaussian rasterization compute pipeline.
        std::vector<VkDescriptorSetLayout> gsLayouts;
        for (const auto& descSet : m_globalDescSets) gsLayouts.push_back(descSet->getSetLayout());
        gsLayouts.push_back(m_gsDescSet->getSetLayout());

        // Create list of descriptor set layouts for post pipeline.
        std::vector<VkDescriptorSetLayout> postLayouts{ m_postDescSet->getSetLayout() };

        // Create pipelines.
        m_preprocessPipeline = std::make_unique<ComputePipeline>(m_device, "gaussian_preprocess", gsLayouts, sizeof(GaussianPreprocessPushConstant));
        m_rasterizePipeline = std::make_unique<ComputePipeline>(m_device, "gaussian_rasterize", gsLayouts, sizeof(GaussianRasterizePushConstant));
        m_postPipeline = std::make_unique<PostPipeline>(m_device, "postShader", postLayouts, m_renderPass, m_swapchain.extent);
    }

    void GaussianRenderer::InitDescriptorSets(Scene& scene) {
        // Initialize itermediary buffers.
        VkDeviceSize geomSize = (num_gaussians * sizeof(Gaussian)) + sizeof(uint32_t);
        ResourceAllocator::createBuffer(geomSize, m_geomBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
        Debugger::setObjectName(m_geomBuffer.buffer, "[Buffer] Gaussian Geometry");
        VkDeviceSize keysSize = (num_gaussians + 1) * sizeof(uint32_t);
        ResourceAllocator::createBuffer(geomSize, m_keysBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
        Debugger::setObjectName(m_keysBuffer.buffer, "[Buffer] Gaussian Keys");

        // Initialize descriptors for every frames in flight.
        VkDescriptorImageInfo imageDescInfos[MAX_FRAMES_IN_FLIGHT];
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // Upload ray-tracing render pass output image uniform.
            std::unique_ptr<Texture> outTexture = std::make_unique<Texture>(m_swapchain.extent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            EngineContext::GetInstance().transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            m_gsDescTextures.push_back(std::move(outTexture));

            imageDescInfos[i].sampler = m_gsDescTextures[i]->getSampler();
            imageDescInfos[i].imageView = m_gsDescTextures[i]->getImageView();
            imageDescInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            m_gsDescSet->writeImage(i, 0, imageDescInfos[i]);
            m_postDescSet->writeImage(i, 0, imageDescInfos[i]);
        }
        // Update descriptor sets.
        m_gsDescSet->update();
        m_postDescSet->update();
    }

    void GaussianRenderer::UpdateDescriptorSets(Scene& scene) {
        // TODO
    }

    //***************************************************************************************//
    //                                       Rendering                                       //
    //***************************************************************************************//

    void GaussianRenderer::RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_preprocessPipeline->GetHandle());

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> preprocessDescSets{ m_globalDescSets[0]->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_preprocessPipeline->GetLayout(), 0, static_cast<uint32_t>(preprocessDescSets.size()), preprocessDescSets.data(), 0, nullptr);

        // Upload push constants
        GaussianPreprocessPushConstant preprocessConstant;
        preprocessConstant.geomAddress = m_geomBuffer.getDeviceAddress();
        preprocessConstant.keysAddress = m_keysBuffer.getDeviceAddress();
        vkCmdPushConstants(commandBuffer, m_preprocessPipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, preprocessConstant.getSize(), &preprocessConstant);

        // Compute Gaussian Preprocessing
        vkCmdDispatch(commandBuffer, (num_gaussians + BLOCK_SIZE) / BLOCK_SIZE, 1, 1);

        // Create memory barrier for geometry buffer.
        VkBufferMemoryBarrier geomBarrier{};
        geomBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        geomBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        geomBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        geomBarrier.buffer = m_geomBuffer.buffer;
        geomBarrier.size = VK_WHOLE_SIZE;
        geomBarrier.offset = 0;
        // Create memory barrier for keys buffer.
        VkBufferMemoryBarrier keysBarrier{};
        keysBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        keysBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        keysBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        keysBarrier.buffer = m_keysBuffer.buffer;
        keysBarrier.size = VK_WHOLE_SIZE;
        keysBarrier.offset = 0;

        // Wait for previous compute shader to finish writing to buffers.
        std::vector<VkBufferMemoryBarrier> barriers{ geomBarrier, keysBarrier };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data(), 0, nullptr);

        // Bind pipeline.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_rasterizePipeline->GetHandle());

        // Bind descriptor sets.
        std::vector<VkDescriptorSet> rasterizeDescSets{ m_globalDescSets[0]->getHandle(currentFrame), m_gsDescSet->getHandle(currentFrame) };
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_rasterizePipeline->GetLayout(), 0, static_cast<uint32_t>(rasterizeDescSets.size()), rasterizeDescSets.data(), 0, nullptr);

        // Upload push constants
        GaussianRasterizePushConstant rasterizeConstant;
        rasterizeConstant.resolution = glm::uvec2(m_swapchain.extent.width, m_swapchain.extent.height);
        rasterizeConstant.geomAddress = m_geomBuffer.getDeviceAddress();
        rasterizeConstant.keysAddress = m_keysBuffer.getDeviceAddress();
        vkCmdPushConstants(commandBuffer, m_rasterizePipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, rasterizeConstant.getSize(), &rasterizeConstant);

        // Compute Gaussian Rasterization
        vkCmdDispatch(commandBuffer, (m_swapchain.extent.width + BLOCK_X) / BLOCK_X, (m_swapchain.extent.height + BLOCK_Y) / BLOCK_Y, 1);

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