#include <pipeline/pipeline.h>

#include <iostream>
#include <fstream>

namespace core {

	Pipeline::Pipeline(VkDevice device, std::vector<DescriptorSet*> descriptorSets) {
		this->type = PipelineType::PIPELINE_TYPE_NONE;
		this->device = device;
		this->descriptorSets = descriptorSets;
	}

	std::vector<VkDescriptorSet> Pipeline::getDescriptorSetHandles() { 
		std::vector<VkDescriptorSet> handles;
		for (auto set : descriptorSets) {
			handles.push_back(set->getHandle());
		}
		return handles; 
	}

	VkShaderModule Pipeline::createShaderModule(const std::string& filename) {
		// Read all file data into a buffer.
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Could not open shader file '" + filename + "'.");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> code(fileSize);

		file.seekg(0);
		file.read(code.data(), fileSize);
		file.close();
		
		// Generate shader module create info.
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// Create shader module.
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Could not create shader module.");
		}
		return shaderModule;
	}
}