#include <debugger.h>

namespace core {

	VkDevice Debugger::device;

	void Debugger::setup(VkDevice device) { 
		Debugger::device = device; 
	}

	void Debugger::setObjectName(const uint64_t handle, const std::string& name, VkObjectType type) {
		VkDebugUtilsObjectNameInfoEXT debugNameInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, type, handle, name.c_str()};

		if (vkSetDebugUtilsObjectNameEXT(device, &debugNameInfo) != VK_SUCCESS) {
			throw std::runtime_error("Debugger failed to set object name.");
		}
	}

	void Debugger::beginCommandLabel(const VkCommandBuffer& commandBuffer, const std::string& label) {
		VkDebugUtilsLabelEXT debugLabel{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, label.c_str(), {1.0f, 1.0f, 1.0f, 1.0f}};

		vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &debugLabel);
	}

	void Debugger::endCommandLabel(const VkCommandBuffer& commandBuffer) {
		vkCmdEndDebugUtilsLabelEXT(commandBuffer);
	}
}
