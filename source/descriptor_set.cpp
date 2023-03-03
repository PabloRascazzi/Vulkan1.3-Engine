#include <descriptor_set.h>
#include <engine_context.h>

namespace core {

	uint32_t DescriptorSet::currentFrame = 0;

	DescriptorSet::~DescriptorSet() {
		cleanup();
	}

	void DescriptorSet::cleanup() {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	void DescriptorSet::addBinding(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStageFlags) {
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = type;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = shaderStageFlags;

		bindings.push_back(layoutBinding);
	}

	void DescriptorSet::create(VkDevice device) {
		this->device = device;
		createLayout();
		createPool();
		allocateDescriptorSets();
	}

	void DescriptorSet::createLayout() {
		// Create DescriptorSet Layout.
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}
	}

	void DescriptorSet::createPool() {
		// Create Descriptor Pool.
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (auto binding : bindings) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = binding.descriptorType;
			poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * poolSizes.size());

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool.");
        }
	}
	
	void DescriptorSet::allocateDescriptorSets() {
		// Allocate Descriptor Sets.
        std::vector<VkDescriptorSetLayout> setLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = setLayouts.data();

		descriptorSets.resize(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets.");
        }
	}

	void DescriptorSet::writeBuffer(uint32_t binding, VkDescriptorBufferInfo& writeDescInfo) {
		VkWriteDescriptorSet bufferWrite{};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bufferWrite.descriptorCount = 1;
		bufferWrite.dstBinding = binding;
		bufferWrite.dstSet = descriptorSets[currentFrame];
		bufferWrite.pBufferInfo = &writeDescInfo;

		writes.push_back(bufferWrite);
	}

	void DescriptorSet::writeImage(uint32_t binding, VkDescriptorImageInfo& writeDescInfo) {
        VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        imageWrite.descriptorCount = 1;
        imageWrite.dstBinding = binding;
        imageWrite.dstSet = descriptorSets[currentFrame];
        imageWrite.pImageInfo = &writeDescInfo;

		writes.push_back(imageWrite);
	}

	void DescriptorSet::writeAccelerationStructureKHR(uint32_t binding, VkWriteDescriptorSetAccelerationStructureKHR writeDescInfo) {
        VkWriteDescriptorSet tlasWrite{};
        tlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        tlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        tlasWrite.descriptorCount = writeDescInfo.accelerationStructureCount;
        tlasWrite.dstBinding = binding;
        tlasWrite.dstSet = descriptorSets[currentFrame];
        tlasWrite.pNext = &writeDescInfo;

		writes.push_back(tlasWrite);
	}

	void DescriptorSet::update() {
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
		writes.clear();
	}
}