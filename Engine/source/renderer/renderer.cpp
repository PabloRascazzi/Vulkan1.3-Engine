#include <renderer/renderer.h>
#include <engine_context.h>

namespace core {

    Renderer::Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& swapchain) :
        m_device(device), m_graphicsQueue(graphicsQueue), m_presentQueue(presentQueue), m_swapchain(swapchain), m_firstRender(true) {
    
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    Renderer::~Renderer() {
        // Cleanup sync objects.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
            m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
            m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
            m_inFlightFences[i] = VK_NULL_HANDLE;
        }
        // Cleanup framebuffers.
        for (auto framebuffer : m_swapchainFramebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }
        // Cleanup renderpass.
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

	void Renderer::CreateCommandBuffers() {
		m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        EngineContext::GetInstance().createCommandBuffer(m_commandBuffers.data(), MAX_FRAMES_IN_FLIGHT);
	}

	void Renderer::CreateSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start fence in signaled state.

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
			VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]));
		}
	}

	void Renderer::Render(const uint32_t currentFrame, Scene& scene) {
        // Initialize descriptor sets on first render.
        if (m_firstRender) {
            InitDescriptorSets(scene);
            m_firstRender = false;
        }

        // Update descriptor sets.
        UpdateDescriptorSets(scene);

        // Wait until previous frame has finished.
        VK_CHECK(vkWaitForFences(m_device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(m_device, 1, &m_inFlightFences[currentFrame]));

        // Acquire next image from swap chain.
        uint32_t imageIndex;
        VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain.handle, UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex));

        // Reset command buffer.
        VK_CHECK(vkResetCommandBuffer(m_commandBuffers[currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        // Record command buffer.
        RecordCommandBuffer(m_commandBuffers[currentFrame], currentFrame, imageIndex, scene);

        // Submit command buffer.
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK_MSG(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[currentFrame]), "Failed to submit render command buffer.");

        // Presentation.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains[] = { m_swapchain.handle };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VK_CHECK(vkQueuePresentKHR(m_presentQueue, &presentInfo));
	}
}