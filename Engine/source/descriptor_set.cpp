#include <descriptor_set.h>
#include <engine_globals.h>
#include <debugger.h>

namespace core {

	bool validateBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings);

	DescriptorSet::~DescriptorSet() {
		cleanup();
	}

	void DescriptorSet::cleanup() {
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
		vkDestroyDescriptorPool(device, pool, nullptr);
	}

	void DescriptorSet::addDescriptor(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStageFlags, VkDescriptorBindingFlags flags) {
		// Check that there is no dynamic descriptor before.
		DEBUG_ASSERT(!this->hasVariableDescriptor);
		
		// Set hasVariableDescriptor flag if the descriptor has flags VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT.
		if ((flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) == VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) this->hasVariableDescriptor = true;

		// Create descriptor set layout binding.
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = type;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = shaderStageFlags;

		// Resize bindings list if too small.
		if (this->bindings.size() - 1 < binding || this->bindings.size() == 0) {
			this->bindings.resize(binding + 1);
			this->bindingFlags.resize(binding + 1);
		}
		// Add new descriptor set layout binding to list of bindings.
		this->bindings[binding] = layoutBinding;
		this->bindingFlags[binding] = flags;
	}

	void DescriptorSet::create(VkDevice device, uint32_t descriptorSetCount) {
		this->device = device;
		this->descriptorSetCount = descriptorSetCount;
		DEBUG_ASSERT(validateBindings(this->bindings));
		createLayout();
		createPool();
		allocateDescriptorSets();
	}

	void DescriptorSet::createLayout() {
		// Create descriptor set layout binding flags.
		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
		flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		flagsInfo.bindingCount = static_cast<uint32_t>(this->bindingFlags.size());
		flagsInfo.pBindingFlags = this->bindingFlags.data();
		
		// Create DescriptorSet Layout.
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(this->bindings.size());
		layoutInfo.pBindings = this->bindings.data();
		layoutInfo.pNext = &flagsInfo;

		VK_CHECK_MSG(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout), "Failed to create descriptor set layout.");
	}

	void DescriptorSet::createPool() {
		// Create Descriptor Pool.
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const auto& binding : this->bindings) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = binding.descriptorType;
			poolSize.descriptorCount = binding.descriptorCount;
			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = this->descriptorSetCount;

		VK_CHECK_MSG(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool), "Failed to create descriptor pool.");
	}
	
	void DescriptorSet::allocateDescriptorSets() {
		// Allocate Descriptor Sets.
		std::vector<uint32_t> varDescCounts;
		std::vector<VkDescriptorSetLayout> setLayouts(this->descriptorSetCount, this->layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = this->pool;
		allocInfo.descriptorSetCount = this->descriptorSetCount;
		allocInfo.pSetLayouts = setLayouts.data();
		if (this->hasVariableDescriptor) {
			// Create variable descriptor count allocate info.
			varDescCounts = std::vector<uint32_t>(this->descriptorSetCount, bindings[bindings.size() - 1].descriptorCount);
			VkDescriptorSetVariableDescriptorCountAllocateInfo variableAllocInfo{};
			variableAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			variableAllocInfo.descriptorSetCount = this->descriptorSetCount;
			variableAllocInfo.pDescriptorCounts = varDescCounts.data();
			// Add variable descriptor Count to descriptor set allocate info.
			allocInfo.pNext = &variableAllocInfo;
		}

		handles.resize(this->descriptorSetCount);
		VK_CHECK_MSG(vkAllocateDescriptorSets(device, &allocInfo, handles.data()), "Failed to allocate descriptor sets.");
	}

	void DescriptorSet::writeBuffer(uint32_t set, uint32_t binding, VkDescriptorBufferInfo& writeDescInfo) {
		// Validate set index.
		DEBUG_ASSERT(set < this->descriptorSetCount);

		// Create descriptor set write.
		VkWriteDescriptorSet bufferWrite{};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.descriptorType = bindings[binding].descriptorType;
		bufferWrite.descriptorCount = 1;
		bufferWrite.dstBinding = binding;
		bufferWrite.dstSet = handles[set];
		bufferWrite.pBufferInfo = &writeDescInfo;

		writes.push_back(bufferWrite);
	}

	void DescriptorSet::writeImage(uint32_t set, uint32_t binding, VkDescriptorImageInfo& writeDescInfo, uint32_t arrayElement) {	
		// Validate set index.
		DEBUG_ASSERT(set < this->descriptorSetCount);

		// Create descriptor set write.
		VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.descriptorType = bindings[binding].descriptorType;
        imageWrite.descriptorCount = 1;
        imageWrite.dstBinding = binding;
		imageWrite.dstSet = handles[set];
		imageWrite.pImageInfo = &writeDescInfo;
		imageWrite.dstArrayElement = arrayElement;

		writes.push_back(imageWrite);
	}

	void DescriptorSet::writeAccelerationStructureKHR(uint32_t set,  uint32_t binding, VkWriteDescriptorSetAccelerationStructureKHR writeDescInfo) {
		// Validate set index.
		DEBUG_ASSERT(set < this->descriptorSetCount);
		
		// Create descriptor set write.
		VkWriteDescriptorSet tlasWrite{};
        tlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        tlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        tlasWrite.descriptorCount = writeDescInfo.accelerationStructureCount;
        tlasWrite.dstBinding = binding;
		tlasWrite.dstSet = handles[set];
        tlasWrite.pNext = &writeDescInfo;

		writes.push_back(tlasWrite);
	}

	void DescriptorSet::update() {
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
		writes.clear();
	}

	void DescriptorSet::setName(const std::string& name) {
		Debugger::setObjectName(this->layout, "[DescriptorSetLayout] " + name);
		Debugger::setObjectName(this->pool, "[DescriptorPool] " + name);
		if (this->descriptorSetCount == 1) {
			Debugger::setObjectName(handles[0], "[DescriptorSet] " + name);
		} else {
			for (uint32_t i = 0; i < this->descriptorSetCount; i++) {
				Debugger::setObjectName(handles[i], "[DescriptorSet] " + name + " (" + std::to_string(i) + ")");
			}
		}
	}

	bool validateBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings) {
		for (uint32_t i = 0; i < bindings.size(); i++) {
			if (bindings[i].binding != i) return false;
		}
		return true;
	}
}