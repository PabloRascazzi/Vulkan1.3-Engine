#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>

namespace core {

	class DescriptorSet {
	public:
		void addBinding(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStageFlags);
		void create(VkDevice device);

		void writeBuffer(uint32_t binding, VkDescriptorBufferInfo& writeDescInfo);
		void writeImage(uint32_t binding, VkDescriptorImageInfo& writeDescInfo);
		void writeAccelerationStructureKHR(uint32_t binding, VkWriteDescriptorSetAccelerationStructureKHR writeDescInfo);
		void update();

		VkDescriptorSetLayout& getSetLayout() { return descriptorSetLayout; }
		VkDescriptorPool& getPool() { return descriptorPool; }
		VkDescriptorSet& getSet() { return descriptorSets[currentFrame]; }

		static void setCurrentFrame(uint32_t index) { currentFrame = index; }

	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkWriteDescriptorSet> writes;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		VkDevice device;

		void createLayout();
		void createPool();
		void allocateDescriptorSets();

		static uint32_t currentFrame;
	};
}
