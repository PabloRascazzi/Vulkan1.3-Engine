#include <texture.h>

namespace core {

	Texture::Texture(VkExtent2D extent, void* data, const VkFormat format, const VkSamplerAddressMode samplerAddressMode, const uint32_t mipLevels, const bool enableAnisotropy, const VkImageUsageFlags& usage) {
		this->extent = extent;
		this->format = format;
		this->samplerAddressMode = samplerAddressMode;
		this->mipLevels = mipLevels;
		this->anisotropyEnabled = enableAnisotropy;
		
		if (data == nullptr) {
			ResourceAllocator::createImage2D(extent, format, mipLevels, image, usage);
		} else {
			ResourceAllocator::createAndStageImage2D(extent, format, mipLevels, data, image, usage);
		}
		ResourceAllocator::createImageView2D(format, image.image, view);
		ResourceAllocator::createSampler2D(samplerAddressMode, enableAnisotropy, image.image, sampler);
	}

	Texture::~Texture() {
		cleanup();
	}

	void Texture::cleanup() {
		ResourceAllocator::destroyImage(this->image);
		ResourceAllocator::destroyImageView(this->view);
		ResourceAllocator::destroySampler(this->sampler);
	}
}