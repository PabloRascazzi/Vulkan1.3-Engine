#pragma once
#include <engine_context.h>
#include <scene.h>
#include <descriptor_set.h>
#include <renderer/renderer.h>
#include <renderer/standard_renderer.h>
#include <renderer/pathtraced_renderer.h>
#include <renderer/gaussian_renderer.h>
#include <swapchain.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class EngineRenderer {
	public:
		EngineRenderer(const std::shared_ptr<Scene>& scene);
		~EngineRenderer();
		void Render(const uint8_t rendermode);
		void RenderToImage(Image& image); // TODO - render to an image instead of a swapchain framebuffer.

		void SetScene(const std::shared_ptr<Scene>& scene) { this->m_scene = scene; }

	private:
		EngineContext& m_context;

		// Swapchain for the main window surface.
		Swapchain m_swapchain;
		uint32_t m_currentSwapchainIndex;

		// Current scene to render.
		std::shared_ptr<Scene> m_scene;
		
		// Descriptor Sets.
		std::shared_ptr<DescriptorSet> m_cameraDescSet;
		std::shared_ptr<DescriptorSet> m_texturesDescSet;

		// Descriptor buffers.
		Buffer m_cameraDescBuffer;
		uint32_t m_cameraDescBufferAlignment;

		// Instances for all the renderers.
		std::unique_ptr<StandardRenderer> m_standardRenderer;
		std::unique_ptr<Renderer> m_raytracedRenderer;
		std::unique_ptr<PathTracedRenderer> m_pathtracedRenderer;
		std::unique_ptr<GaussianRenderer> m_gaussianRenderer;

		void CreateSwapChain();
		void CreateRenderers();
		void CreateDescriptorSets();
		void InitDescriptorSets();

		// Recreates the swapchain.
		void UpdateSwapchain();
		// Updates the global descriptor sets' buffer data.
		void UpdateDescriptorSets();
		void UpdateCameraDescriptor();
		void UpdateTextureArrayDescriptor();

	};
}