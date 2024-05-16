#pragma once
#include <engine_globals.h>
#include <swapchain.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class Renderer {
	public:
		Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, Swapchain& m_swapchain);
		~Renderer();

		void Render(const uint32_t currentFrame, Scene& scene);

		VkRenderPass& GetRenderPass() { return m_renderPass; }

	private:
		void CreateCommandBuffers();
		void CreateSyncObjects();

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

		bool m_firstRender;

	protected:
		virtual void CreateRenderPass() = 0;
		virtual void CreateFramebuffers() = 0;
		virtual void CreateDescriptorSets() = 0;
		virtual void CreatePipeline() = 0;

		virtual void InitDescriptorSets(Scene& scene) = 0;
		virtual void UpdateDescriptorSets(Scene& scene) = 0;

		virtual void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t currentFrame, uint32_t imageIndex, Scene& scene) = 0;

		VkDevice m_device;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkRenderPass m_renderPass;

		Swapchain& m_swapchain;
		std::vector<VkFramebuffer> m_swapchainFramebuffers;
		std::vector<VkCommandBuffer> m_commandBuffers;

	};
}