#include <pipeline/pipeline.h>

#include <iostream>
#include <fstream>

namespace core {

	Pipeline::Pipeline(VkDevice device, const PipelineType& type) : 
		m_type(type), m_device(device), m_pipeline(VK_NULL_HANDLE), m_layout(VK_NULL_HANDLE) {}

	Pipeline::~Pipeline() {
		if (m_pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(m_device, m_pipeline, nullptr);
			m_pipeline = VK_NULL_HANDLE;
		}
		if (m_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(m_device, m_layout, nullptr);
			m_layout = VK_NULL_HANDLE;
		}
	}

	VkShaderModule Pipeline::CreateShaderModule(const std::string& shadername) {
		// Read all file data into a buffer.
		std::ifstream file(shadername, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Could not open shader file '" + shadername + "'.");
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
		if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Could not create shader module.");
		}
		return shaderModule;
	}
}