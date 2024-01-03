#pragma once
#include <scene.h>
#include <descriptor_set.h>
#include <renderer/renderer.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	struct Swapchain {
		VkSwapchainKHR handle;
		VkFormat format;
		VkExtent2D extent;
		// Swapchain needs one image for each frames in flight.
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		void destroy(VkDevice device) {
			for (auto imageView : this->imageViews) {
				vkDestroyImageView(device, imageView, nullptr);
			}
			vkDestroySwapchainKHR(device, this->handle, nullptr);
		}
	};

	class EngineRenderer {
	public:
		static void setup(Scene* scene);
		static void cleanup();
		static void render(const bool raytrace);
		static void renderToImage(Image& image); // TODO - render to an image instead of a swapchain framebuffer.

		static Swapchain& getSwapchain() { return swapchain; }
		static uint32_t getCurrentSwapchainIndex() { return currentSwapchainIndex; }

		static void setScene(Scene& scene) { EngineRenderer::scene = &scene; }

	private:
		// Swapchain for the main window surface.
		static Swapchain swapchain;
		static uint32_t currentSwapchainIndex;

		// Current scene to render.
		static Scene* scene; 
		
		// Descriptor Sets.
		static DescriptorSet* globalDescSet;

		// Descriptor buffers.
		static std::vector<Buffer> cameraDescBuffers;

		// Instances for all the renderers.
		static Renderer* standardRenderer;
		static Renderer* raytracedRenderer;
		static Renderer* pathtracedRenderer;

		static void createSwapChain();
		static void createRenderers();
		static void createDescriptorSets();
		static void initDescriptorSets();

		// Recreates the swapchain.
		static void updateSwapchain();
		// Updates the global descriptor sets' buffer data.
		static void updateDescriptorSets();
		static void updateCameraDescriptor();
		static void updateTextureArrayDescriptor();

	};
}