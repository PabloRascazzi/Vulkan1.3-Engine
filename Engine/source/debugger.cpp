#include <debugger.h>
#include <engine_globals.h>

namespace core {

	const bool Debugger::debugReportEnabled;
	VkInstance Debugger::instance;
	VkDevice Debugger::device;
	VkDebugReportCallbackEXT Debugger::debugReportCallback;

	void Debugger::setup(VkInstance instance, VkDevice device) { 
		Debugger::instance = instance;
		Debugger::device = device;

		if (debugReportEnabled) {
			createDebugReportCallback(instance);
		}
	}

	void Debugger::cleanup() {
		if (debugReportEnabled) {
			vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
		}
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

	VkBool32 Debugger::debugReportCallbackHandler(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,  size_t location,  int32_t messageCode, const char* pLayerPrefix, const char* pMessage,  void* pUserData) {
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            printf("Debugger [Info]: %s\n", pMessage);
        }
        return false;
	}

	void Debugger::createDebugReportCallback(VkInstance instance) {
		VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo{};
		debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugCallbackCreateInfo.pfnCallback = debugReportCallbackHandler;
		debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		debugCallbackCreateInfo.pUserData = nullptr;

		VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &debugCallbackCreateInfo, nullptr, &debugReportCallback));
	}
}
