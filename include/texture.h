#pragma once
#include <Vulkan/vulkan.hpp>
#include <resource_allocator.h>

namespace core {

	class Texture {
	public:
		Texture(VkExtent2D extent, void* data, const VkFormat format, const VkSamplerAddressMode samplerAddressMode, const uint32_t mipLevels, const bool enableAnisotropy, const VkImageUsageFlags& usage);
		~Texture();

		void cleanup();

		Image& getImage() { return image; }
		VkImageView& getImageView() { return view; }
		VkSampler& getSampler() { return sampler; }

		VkExtent2D& getExtent() { return extent; }
		VkFormat& getFormat() { return format; }
		VkSamplerAddressMode& getSamplerAddressMode() { return samplerAddressMode; }
		uint32_t getMipLevels() { return mipLevels; }
		bool isAnisotropyEnabled() { return anisotropyEnabled; }

	private:
		Image image;
		VkImageView view;
		VkSampler sampler;

		VkExtent2D extent;
		VkFormat format;
		VkSamplerAddressMode samplerAddressMode;
		uint32_t mipLevels;
		bool anisotropyEnabled;

	};
}