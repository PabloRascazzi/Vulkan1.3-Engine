#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include <array>

namespace core {

	class Debugger {
	public:
		static const bool debugReportEnabled = false; // Boolean for if debug report is enabled.

		// Must be called once after the instance and the device are created.
		static void setup(VkInstance instance, VkDevice device);
		static void cleanup();

		// Sets object name for debugging.
		static void setObjectName(const uint64_t handle, const std::string& name, VkObjectType type);
		// Groups commands under a debug label.
		static void beginCommandLabel(const VkCommandBuffer& commandBuffer, const std::string& label);
		static void endCommandLabel(const VkCommandBuffer& commandBuffer);

		// Set object name helpers.
		static void setObjectName(const VkInstance& object, const std::string& name)                      { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_INSTANCE); }
		static void setObjectName(const VkPhysicalDevice &object, const std::string& name)                { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PHYSICAL_DEVICE); }
		static void setObjectName(const VkDevice &object, const std::string& name)                        { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE); }
		static void setObjectName(const VkQueue &object, const std::string& name)                         { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUEUE); }
		static void setObjectName(const VkSemaphore &object, const std::string& name)                     { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SEMAPHORE); }
		static void setObjectName(const VkCommandBuffer &object, const std::string& name)                 { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_BUFFER); }
		static void setObjectName(const VkFence &object, const std::string& name)                         { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_FENCE); }
		static void setObjectName(const VkDeviceMemory &object, const std::string& name)                  { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE_MEMORY); }
		static void setObjectName(const VkBuffer &object, const std::string& name)                        { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER); }
		static void setObjectName(const VkImage &object, const std::string& name)                         { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE); }
		static void setObjectName(const VkEvent &object, const std::string& name)                         { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_EVENT); }
		static void setObjectName(const VkQueryPool &object, const std::string& name)                     { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUERY_POOL); }
		static void setObjectName(const VkBufferView &object, const std::string& name)                    { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER_VIEW); }
		static void setObjectName(const VkImageView &object, const std::string& name)                     { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE_VIEW); }
		static void setObjectName(const VkShaderModule &object, const std::string& name)                  { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SHADER_MODULE); }
		static void setObjectName(const VkPipelineCache &object, const std::string& name)                 { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE_CACHE); }
		static void setObjectName(const VkPipelineLayout &object, const std::string& name)                { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE_LAYOUT); }
		static void setObjectName(const VkRenderPass &object, const std::string& name)                    { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_RENDER_PASS); }
		static void setObjectName(const VkPipeline &object, const std::string& name)                      { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE); }
		static void setObjectName(const VkDescriptorSetLayout &object, const std::string& name)           { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT); }
		static void setObjectName(const VkSampler &object, const std::string& name)                       { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SAMPLER); }
		static void setObjectName(const VkDescriptorPool &object, const std::string& name)                { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_POOL); }
		static void setObjectName(const VkDescriptorSet &object, const std::string& name)                 { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET); }
		static void setObjectName(const VkFramebuffer &object, const std::string& name)                   { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_FRAMEBUFFER); }
		static void setObjectName(const VkCommandPool &object, const std::string& name)                   { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_POOL); }
		static void setObjectName(const VkSamplerYcbcrConversion &object, const std::string& name)        { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION); }
		static void setObjectName(const VkDescriptorUpdateTemplate &object, const std::string& name)      { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE); }
		static void setObjectName(const VkPrivateDataSlot &object, const std::string& name)               { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PRIVATE_DATA_SLOT); }
		static void setObjectName(const VkSurfaceKHR &object, const std::string& name)                    { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SURFACE_KHR); }
		static void setObjectName(const VkSwapchainKHR &object, const std::string& name)                  { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SWAPCHAIN_KHR); }
		static void setObjectName(const VkDisplayKHR &object, const std::string& name)                    { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DISPLAY_KHR); }
		static void setObjectName(const VkDisplayModeKHR &object, const std::string& name)                { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DISPLAY_MODE_KHR); }
		static void setObjectName(const VkDebugReportCallbackEXT &object, const std::string& name)        { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT); }
		static void setObjectName(const VkCuModuleNVX &object, const std::string& name)                   { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_CU_MODULE_NVX); }
		static void setObjectName(const VkCuFunctionNVX &object, const std::string& name)                 { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_CU_FUNCTION_NVX); }
		static void setObjectName(const VkDebugUtilsMessengerEXT &object, const std::string& name)        { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT); }
		static void setObjectName(const VkAccelerationStructureKHR &object, const std::string& name)      { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR); }
		static void setObjectName(const VkValidationCacheEXT &object, const std::string& name)            { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_VALIDATION_CACHE_EXT); }
		static void setObjectName(const VkAccelerationStructureNV &object, const std::string& name)       { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV); }
		static void setObjectName(const VkPerformanceConfigurationINTEL &object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL); }
		static void setObjectName(const VkDeferredOperationKHR &object, const std::string& name)          { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR); }
		static void setObjectName(const VkIndirectCommandsLayoutNV &object, const std::string& name)      { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV); }
		static void setObjectName(const VkMicromapEXT &object, const std::string& name)                   { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_MICROMAP_EXT); }
		static void setObjectName(const VkOpticalFlowSessionNV &object, const std::string& name)          { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV); }

	private:
		static VkInstance instance;
		static VkDevice device;
		static VkDebugReportCallbackEXT debugReportCallback;

		static void createDebugReportCallback(VkInstance instance);
		static VkBool32 debugReportCallbackHandler(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

	};
}
