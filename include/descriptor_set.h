#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>

namespace core {

	class DescriptorSet {
	public:
		~DescriptorSet();
		void cleanup();

		void addDescriptor(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStageFlags, VkDescriptorBindingFlags flags = 0);
		void create(VkDevice device, uint32_t descriptorSetCount = 1);

		void writeBuffer(uint32_t set, uint32_t binding, VkDescriptorBufferInfo& writeDescInfo);
		void writeImage(uint32_t set, uint32_t binding, VkDescriptorImageInfo& writeDescInfo, uint32_t arrayElement = 0);
		void writeAccelerationStructureKHR(uint32_t set, uint32_t binding, VkWriteDescriptorSetAccelerationStructureKHR writeDescInfo);
		void update();
		void setName(const std::string& name);

		VkDescriptorSetLayout& getSetLayout() { return layout; }
		VkDescriptorPool& getPool() { return pool; }
		VkDescriptorSet& getHandle(uint32_t set) { return handles[set]; }

	private:
		std::vector<VkDescriptorSet> handles;
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		VkDevice device;

		uint32_t descriptorSetCount;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorBindingFlags> bindingFlags;
		std::vector<VkWriteDescriptorSet> writes;
		bool hasVariableDescriptor;

		void createLayout();
		void createPool();
		void allocateDescriptorSets();

	};
}
