#include <pipeline/pipeline.h>

#include <iostream>
#include <fstream>

namespace core {

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