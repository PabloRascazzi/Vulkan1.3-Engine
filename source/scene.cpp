#include <scene.h>
#include <engine_context.h>
#include <iostream>

namespace core {

	Scene::Scene() {}

	Scene::~Scene() {
		cleanup();
	}

	void Scene::setup() {
		buildAS(objects);
	}

	void Scene::cleanup() {}

	Object* Scene::addObject(Mesh* mesh, glm::mat4 transform, uint32_t shader) {
		Object obj = {mesh, transform, shader};
		objects.push_back(obj);
		return &obj;
	}

	struct BlasCreateInfo {
		VkAccelerationStructureGeometryKHR geometry;
		VkAccelerationStructureBuildRangeInfoKHR offset;
		BottomLevelAccelerationStructure* pBLAS;
	};

	void buildBLAS(std::vector<BlasCreateInfo>& input);
	BlasCreateInfo meshToBlasCreateInfo(Mesh& mesh);

	void Scene::buildAS(std::vector<Object>& objects) {
		// Create Bottom Level Acceleratino Structure.
		std::vector<BlasCreateInfo> allBlasCreateInfo;
		allBlasCreateInfo.reserve(objects.size());
		for (auto &obj : objects) {
			allBlasCreateInfo.emplace_back(meshToBlasCreateInfo(*obj.mesh));
		}
		buildBLAS(allBlasCreateInfo);
		std::cout << objects[0].mesh->getBLAS().handle << std::endl;
	}

	BlasCreateInfo meshToBlasCreateInfo(Mesh& mesh) {
		VkDeviceAddress vertexAddress = EngineContext::getBufferDeviceAddress(mesh.getVertexBuffer());
		VkDeviceAddress indexAddress = EngineContext::getBufferDeviceAddress(mesh.getIndexBuffer());
		uint32_t maxPrimitiveCount = mesh.getIndexCount()/3;
		
		VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vertexAddress;
		triangles.vertexStride = sizeof(Vertex);
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = indexAddress;
		triangles.maxVertex = mesh.getVertexCount();

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.geometry.triangles = triangles;

		VkAccelerationStructureBuildRangeInfoKHR offset{};
		offset.primitiveCount = maxPrimitiveCount;
		offset.primitiveOffset = 0;
		offset.firstVertex = 0;
		offset.transformOffset = 0;

		return BlasCreateInfo{geometry, offset, &mesh.getBLAS() };
	}

	void buildBLAS(std::vector<BlasCreateInfo>& input) {
		uint32_t blasCount = input.size();
		uint32_t campactionCount = 0; // Number of BLAS requesting compaction.
		VkDeviceSize totalSize = 0; // Memory size of all allocated BLAS.
		VkDeviceSize maxScratchSize = 0; // Largest scratch size.

		struct AccelerationStructureBuildGeometryInfo {
			VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
			const VkAccelerationStructureBuildRangeInfoKHR* rangeInfo;
			VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
		};

		std::vector<AccelerationStructureBuildGeometryInfo> buildAS(blasCount);
		for (uint32_t i = 0; i < blasCount; i++) {
			// Fill Acceleration Structure Geometry Build Info.
			buildAS[i].buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			buildAS[i].buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			buildAS[i].buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			buildAS[i].buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; // Optional: add VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR for all BLAS that require compaction.
			buildAS[i].buildInfo.geometryCount = 1;
			buildAS[i].buildInfo.pGeometries = &(input[i].geometry);
			// Fill Acceleration Structure Geometry Range Info.
			buildAS[i].rangeInfo = &input[i].offset;

			// Find sizes for accelerated structure and scratch buffer.
			vkGetAccelerationStructureBuildSizesKHR(EngineContext::getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildAS[i].buildInfo, &buildAS[i].rangeInfo[0].primitiveCount, &buildAS[i].sizeInfo);
		
			totalSize += buildAS[i].sizeInfo.accelerationStructureSize;
			maxScratchSize += std::max(maxScratchSize, buildAS[i].sizeInfo.buildScratchSize);
		}

		// Allocate scratch buffers holding the temporary data of the acceleration structure builder.
		VkBuffer scratchBuffer;
		VmaAllocation scratchBufferAlloc;
		EngineContext::createBuffer(maxScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, scratchBuffer, scratchBufferAlloc);
		VkDeviceAddress scratchAddress = EngineContext::getBufferDeviceAddress(scratchBuffer);

		// Batch creation of BLAS to allow staying in restricted amount of memory.
		VkDeviceSize batchSize = 0;
		VkDeviceSize batchLimit = 256'000'000; // 256 MB
		for (uint32_t i = 0; i < blasCount; i++) {
			batchSize += buildAS[i].sizeInfo.accelerationStructureSize;
			
			// Over the batch limit or the last BLAS element.
			if (batchSize >= batchLimit || i == blasCount-1) {
				VkCommandBuffer cmdBuffer;
				EngineContext::createCommandBuffer(&cmdBuffer, 1);

				// Record command buffer.
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				vkBeginCommandBuffer(cmdBuffer, &beginInfo);

				// Actual allocation of buffer and acceleration structure.
				EngineContext::createBuffer(buildAS[i].sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, input[i].pBLAS->buffer, input[i].pBLAS->alloc);
        
				VkAccelerationStructureCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
				createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				createInfo.size = buildAS[i].sizeInfo.accelerationStructureSize;
				createInfo.buffer = input[i].pBLAS->buffer;

				if (vkCreateAccelerationStructureKHR(EngineContext::getDevice(), &createInfo, nullptr, &input[i].pBLAS->handle) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create acceleration structure.");
				}

				buildAS[i].buildInfo.dstAccelerationStructure = input[i].pBLAS->handle;
				buildAS[i].buildInfo.scratchData.deviceAddress = scratchAddress;

				// Building the bottom-level-acceleration-structure
				vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildAS[i].buildInfo, &buildAS[i].rangeInfo);
				
				VkMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
				barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

				// End command buffer.
				vkEndCommandBuffer(cmdBuffer);

				// Submit command buffer and wait.
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmdBuffer;

				vkQueueSubmit(EngineContext::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(EngineContext::getGraphicsQueue());

				// Reset
				batchSize = 0;
			}
		}

		EngineContext::destroyBuffer(scratchBuffer, scratchBufferAlloc);
	}
}
