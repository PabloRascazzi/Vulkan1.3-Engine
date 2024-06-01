#pragma once
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

		void Destroy(VkDevice device) {
			for (auto imageView : this->imageViews) {
				vkDestroyImageView(device, imageView, nullptr);
				imageView = VK_NULL_HANDLE;
			}
			vkDestroySwapchainKHR(device, this->handle, nullptr);
			this->handle = VK_NULL_HANDLE;
		}
	};
}