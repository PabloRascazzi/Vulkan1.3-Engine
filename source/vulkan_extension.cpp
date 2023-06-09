#include <Vulkan/vulkan.hpp>

#ifdef VK_KHR_acceleration_structure
static PFN_vkCreateAccelerationStructureKHR pfn_vkCreateAccelerationStructureKHR = 0;
static PFN_vkDestroyAccelerationStructureKHR pfn_vkDestroyAccelerationStructureKHR = 0;
static PFN_vkCmdBuildAccelerationStructuresKHR pfn_vkCmdBuildAccelerationStructuresKHR = 0;
static PFN_vkCmdBuildAccelerationStructuresIndirectKHR pfn_vkCmdBuildAccelerationStructuresIndirectKHR = 0;
static PFN_vkBuildAccelerationStructuresKHR pfn_vkBuildAccelerationStructuresKHR = 0;
static PFN_vkCopyAccelerationStructureKHR pfn_vkCopyAccelerationStructureKHR = 0;
static PFN_vkCopyAccelerationStructureToMemoryKHR pfn_vkCopyAccelerationStructureToMemoryKHR = 0;
static PFN_vkCopyMemoryToAccelerationStructureKHR pfn_vkCopyMemoryToAccelerationStructureKHR = 0;
static PFN_vkWriteAccelerationStructuresPropertiesKHR pfn_vkWriteAccelerationStructuresPropertiesKHR = 0;
static PFN_vkCmdCopyAccelerationStructureKHR pfn_vkCmdCopyAccelerationStructureKHR = 0;
static PFN_vkCmdCopyAccelerationStructureToMemoryKHR pfn_vkCmdCopyAccelerationStructureToMemoryKHR = 0;
static PFN_vkCmdCopyMemoryToAccelerationStructureKHR pfn_vkCmdCopyMemoryToAccelerationStructureKHR = 0;
static PFN_vkGetAccelerationStructureDeviceAddressKHR pfn_vkGetAccelerationStructureDeviceAddressKHR = 0;
static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR pfn_vkCmdWriteAccelerationStructuresPropertiesKHR = 0;
static PFN_vkGetDeviceAccelerationStructureCompatibilityKHR pfn_vkGetDeviceAccelerationStructureCompatibilityKHR = 0;
static PFN_vkGetAccelerationStructureBuildSizesKHR pfn_vkGetAccelerationStructureBuildSizesKHR = 0;
#endif /* VK_KHR_acceleration_structure */
#ifdef VK_KHR_deferred_host_operations
static PFN_vkCreateDeferredOperationKHR pfn_vkCreateDeferredOperationKHR = 0;
static PFN_vkDestroyDeferredOperationKHR pfn_vkDestroyDeferredOperationKHR = 0;
static PFN_vkGetDeferredOperationMaxConcurrencyKHR pfn_vkGetDeferredOperationMaxConcurrencyKHR = 0;
static PFN_vkGetDeferredOperationResultKHR pfn_vkGetDeferredOperationResultKHR = 0;
static PFN_vkDeferredOperationJoinKHR pfn_vkDeferredOperationJoinKHR = 0;
#endif /* VK_KHR_deferred_host_operations */
#ifdef VK_KHR_ray_tracing_pipeline
static PFN_vkCmdTraceRaysKHR pfn_vkCmdTraceRaysKHR = 0;
static PFN_vkCreateRayTracingPipelinesKHR pfn_vkCreateRayTracingPipelinesKHR = 0;
static PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR pfn_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = 0;
static PFN_vkCmdTraceRaysIndirectKHR pfn_vkCmdTraceRaysIndirectKHR = 0;
static PFN_vkGetRayTracingShaderGroupStackSizeKHR pfn_vkGetRayTracingShaderGroupStackSizeKHR = 0;
static PFN_vkCmdSetRayTracingPipelineStackSizeKHR pfn_vkCmdSetRayTracingPipelineStackSizeKHR = 0;
static PFN_vkGetRayTracingShaderGroupHandlesKHR pfn_vkGetRayTracingShaderGroupHandlesKHR = 0;
#endif /* VK_KHR_ray_tracing_pipeline */
#ifdef VK_EXT_debug_utils
static PFN_vkSetDebugUtilsObjectNameEXT pfn_vkSetDebugUtilsObjectNameEXT = 0;
static PFN_vkSetDebugUtilsObjectTagEXT pfn_vkSetDebugUtilsObjectTagEXT = 0;
static PFN_vkQueueBeginDebugUtilsLabelEXT pfn_vkQueueBeginDebugUtilsLabelEXT = 0;
static PFN_vkQueueEndDebugUtilsLabelEXT pfn_vkQueueEndDebugUtilsLabelEXT = 0;
static PFN_vkQueueInsertDebugUtilsLabelEXT pfn_vkQueueInsertDebugUtilsLabelEXT = 0;
static PFN_vkCmdBeginDebugUtilsLabelEXT pfn_vkCmdBeginDebugUtilsLabelEXT = 0;
static PFN_vkCmdEndDebugUtilsLabelEXT pfn_vkCmdEndDebugUtilsLabelEXT = 0;
static PFN_vkCmdInsertDebugUtilsLabelEXT pfn_vkCmdInsertDebugUtilsLabelEXT = 0;
static PFN_vkCreateDebugUtilsMessengerEXT pfn_vkCreateDebugUtilsMessengerEXT = 0;
static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = 0;
static PFN_vkSubmitDebugUtilsMessageEXT pfn_vkSubmitDebugUtilsMessageEXT = 0;
#endif /* VK_EXT_debug_utils */
#ifdef VK_EXT_debug_report
static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT = 0;
static PFN_vkDebugReportMessageEXT pfn_vkDebugReportMessageEXT = 0;
static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT = 0;
#endif /* VK_EXT_debug_report */


#ifdef VK_KHR_acceleration_structure
VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure)
{
    return pfn_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator)
{
    return pfn_vkDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}
VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
    return pfn_vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts)
{
    return pfn_vkCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
VKAPI_ATTR VkResult VKAPI_CALL vkBuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
    return pfn_vkBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
    return pfn_vkCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
    return pfn_vkCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
    return pfn_vkCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride)
{
    return pfn_vkWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}
VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
    return pfn_vkCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}
VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
    return pfn_vkCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}
VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
    return pfn_vkCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}
VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
    return pfn_vkGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}
VKAPI_ATTR void VKAPI_CALL vkCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery)
{
    return pfn_vkCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
VKAPI_ATTR void VKAPI_CALL vkGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility)
{
    return pfn_vkGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}
VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{
    return pfn_vkGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}
#endif /* VK_KHR_acceleration_structure */
#ifdef VK_KHR_deferred_host_operations
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation)
{
    return pfn_vkCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator)
{
    return pfn_vkDestroyDeferredOperationKHR(device, operation, pAllocator);
}
VKAPI_ATTR uint32_t VKAPI_CALL vkGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation)
{
    return pfn_vkGetDeferredOperationMaxConcurrencyKHR(device, operation);
}
VKAPI_ATTR VkResult VKAPI_CALL vkGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation)
{
    return pfn_vkGetDeferredOperationResultKHR(device, operation);
}
VKAPI_ATTR VkResult VKAPI_CALL vkDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) 
{
    return pfn_vkDeferredOperationJoinKHR(device, operation);
}
#endif /* VK_KHR_deferred_host_operations */
#ifdef VK_KHR_ray_tracing_pipeline
VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth)
{
    return pfn_vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) 
{
    return pfn_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
    return pfn_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}
VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
    return pfn_vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}
VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress)
{
    return pfn_vkCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}
VKAPI_ATTR VkDeviceSize VKAPI_CALL vkGetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader)
{
    return pfn_vkGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
}
VKAPI_ATTR void VKAPI_CALL vkCmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize)
{
    return pfn_vkCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}
#endif /* VK_KHR_ray_tracing_pipeline */
#ifdef VK_EXT_debug_utils
VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) 
{
    return pfn_vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) 
{
    return pfn_vkSetDebugUtilsObjectTagEXT(device, pTagInfo);
}
VKAPI_ATTR void VKAPI_CALL vkQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    return pfn_vkQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}
VKAPI_ATTR void VKAPI_CALL vkQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) 
{
    return pfn_vkQueueEndDebugUtilsLabelEXT(queue);
}
VKAPI_ATTR void VKAPI_CALL vkQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    return pfn_vkQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}
VKAPI_ATTR void VKAPI_CALL vkCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) 
{
    return pfn_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}
VKAPI_ATTR void VKAPI_CALL vkCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) 
{
    return pfn_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}
VKAPI_ATTR void VKAPI_CALL vkCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) 
{
    return pfn_vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) 
{
    return pfn_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) 
{
    return pfn_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
VKAPI_ATTR void VKAPI_CALL vkSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) 
{
    return pfn_vkSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#endif /* VK_EXT_debug_utils */
#ifdef VK_EXT_debug_report
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) 
{
    return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}
VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) 
{
    return pfn_vkDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) 
{
    return pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}
#endif /* VK_EXT_debug_report */

#ifdef VK_KHR_acceleration_structure
void load_extension_VK_KHR_acceleration_structure(VkDevice device) {
    pfn_vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
    pfn_vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
    pfn_vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
    pfn_vkCmdBuildAccelerationStructuresIndirectKHR = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresIndirectKHR");
    pfn_vkBuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR");
    pfn_vkCopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureKHR");
    pfn_vkCopyAccelerationStructureToMemoryKHR = (PFN_vkCopyAccelerationStructureToMemoryKHR)vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureToMemoryKHR");
    pfn_vkCopyMemoryToAccelerationStructureKHR = (PFN_vkCopyMemoryToAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCopyMemoryToAccelerationStructureKHR");
    pfn_vkWriteAccelerationStructuresPropertiesKHR = (PFN_vkWriteAccelerationStructuresPropertiesKHR)vkGetDeviceProcAddr(device, "vkWriteAccelerationStructuresPropertiesKHR");
    pfn_vkCmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR");
    pfn_vkCmdCopyAccelerationStructureToMemoryKHR = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureToMemoryKHR");
    pfn_vkCmdCopyMemoryToAccelerationStructureKHR = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCmdCopyMemoryToAccelerationStructureKHR");
    pfn_vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR");
    pfn_vkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    pfn_vkGetDeviceAccelerationStructureCompatibilityKHR = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)vkGetDeviceProcAddr(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    pfn_vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");
}
#endif /* VK_KHR_acceleration_structure */
#ifdef VK_KHR_deferred_host_operations
void load_extension_VK_KHR_deferred_host_operations(VkDevice device) {
    pfn_vkCreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)vkGetDeviceProcAddr(device, "vkCreateDeferredOperationKHR");
    pfn_vkDestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)vkGetDeviceProcAddr(device, "vkDestroyDeferredOperationKHR");
    pfn_vkGetDeferredOperationMaxConcurrencyKHR = (PFN_vkGetDeferredOperationMaxConcurrencyKHR)vkGetDeviceProcAddr(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    pfn_vkGetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)vkGetDeviceProcAddr(device, "vkGetDeferredOperationResultKHR");
    pfn_vkDeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)vkGetDeviceProcAddr(device, "vkDeferredOperationJoinKHR");
}
#endif /* VK_KHR_deferred_host_operations */
#ifdef VK_KHR_ray_tracing_pipeline
void load_extension_VK_KHR_ray_tracing_pipeline(VkDevice device) {
	pfn_vkCmdSetRayTracingPipelineStackSizeKHR = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)vkGetDeviceProcAddr(device, "vkCmdSetRayTracingPipelineStackSizeKHR");
	pfn_vkCmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysIndirectKHR");
	pfn_vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
	pfn_vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
	pfn_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
	pfn_vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR");
	pfn_vkGetRayTracingShaderGroupStackSizeKHR = (PFN_vkGetRayTracingShaderGroupStackSizeKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupStackSizeKHR");
	pfn_vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
}
#endif /* VK_KHR_ray_tracing_pipeline */
#ifdef VK_EXT_debug_utils
void load_extension_VK_EXT_debug_utils(VkInstance instance) {
    pfn_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    pfn_vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
    pfn_vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
    pfn_vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");
    pfn_vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
    pfn_vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    pfn_vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    pfn_vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
    pfn_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    pfn_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    pfn_vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
}
#endif /* VK_EXT_debug_utils */
#ifdef VK_EXT_debug_report
void load_extension_VK_EXT_debug_report(VkInstance instance) {
    pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    pfn_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
    pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
}
#endif /* VK_EXT_debug_report */