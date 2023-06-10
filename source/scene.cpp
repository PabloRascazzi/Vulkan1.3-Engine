#include <scene.h>
#include <engine_context.h>
#include <engine_globals.h>
#include <iostream>
#include <algorithm>

namespace core {

	Scene::Scene() {}

	Scene::~Scene() {
		cleanup();
	}

	void Scene::setup() {
		buildAccelerationStructure(objects, meshes);
		createObjectDescriptions(objects);
	}

	void Scene::cleanup() {
		// Delete all scene cameras.
		for (auto& camera : cameras) delete camera;
		cameras.clear();

		ResourceAllocator::destroyAccelerationStructure(tlas);
		ResourceAllocator::destroyBuffer(objDescBuffer);
	}

	void Scene::update() {
		// Update all cameras.
		for (auto& camera : cameras) {
			camera->update();
		}
	}

	Camera* Scene::addCamera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n, const float& f) {
		Camera* cam = new Camera(transform, fov, aspectRatio, n, f);
		cameras.push_back(cam);
		if (cameras.size() == 1) this->mainCamera = cameras[0];
		return cameras[cameras.size()-1];
	}

	Object* Scene::addObject(Mesh* mesh, std::vector<Material*> mats, glm::mat4 transform, uint32_t shader) {
		Object obj = {mesh, mats, transform, shader};
		objects.push_back(obj);
		meshes.insert(mesh);
		for (auto material : mats) materials.insert(material);
		return &objects[objects.size()-1];
	}

	void Scene::setMainCamera(Camera* camera) {
		// Check if camera is part of the scene before setting as main camera.
		if (std::find(cameras.begin(), cameras.end(), camera) != cameras.end()) {
			this->mainCamera = camera; 
		}
	}

	void Scene::createObjectDescriptions(std::vector<Object>& objects) {
		for (auto obj : objects) {
			for (uint32_t k = 0; k < obj.mesh->getSubmeshCount() && k < obj.materials.size(); k++) {
				VkDeviceAddress vertexAddress = obj.mesh->getVertexBuffer().getDeviceAddress();
				VkDeviceAddress indexAddress = obj.mesh->getSubmesh(k).getIndexBuffer().getDeviceAddress();
				VkDeviceAddress materialAddress = obj.materials.at(k)->getBuffer().getDeviceAddress();
				ObjDesc desc = {vertexAddress, indexAddress, materialAddress};
				objDescriptions.push_back(desc);
			}
		}

		VkDeviceSize size = sizeof(ObjDesc) * static_cast<uint64_t>(objDescriptions.size());
		ResourceAllocator::createAndStageBuffer(size, objDescriptions.data(), objDescBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	}

    //***************************************************************************************//
    //                             Build Acceleration Structure                              //
    //***************************************************************************************//

	struct BottomLevelAccelerationStructureCreateInfo {
		VkAccelerationStructureGeometryKHR geometry;
		VkAccelerationStructureBuildRangeInfoKHR offset;
		AccelerationStructure* pBLAS;
	};

	std::vector<BottomLevelAccelerationStructureCreateInfo> fetchAllBottomLevelAccelerationStructureCreateInfo(std::unordered_set<Mesh*>& meshes) {
		std::vector<BottomLevelAccelerationStructureCreateInfo> createInfos;
		createInfos.reserve(meshes.size());

		for (auto mesh : meshes) {
			// Fetch vertex buffer address
			VkDeviceAddress vertexAddress = mesh->getVertexBuffer().getDeviceAddress();

			for (uint32_t submeshIndex = 0; submeshIndex < mesh->getSubmeshCount(); submeshIndex++) {
				// Fetch index buffer address
				VkDeviceAddress indexAddress = mesh->getSubmesh(submeshIndex).getIndexBuffer().getDeviceAddress();
				uint32_t maxPrimitiveCount = mesh->getSubmesh(submeshIndex).getIndexCount()/3;
				
				// Create acceleration gtructure geometry triangles data
				VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
				triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
				triangles.vertexData.deviceAddress = vertexAddress;
				triangles.vertexStride = sizeof(Vertex);
				triangles.indexType = VK_INDEX_TYPE_UINT32;
				triangles.indexData.deviceAddress = indexAddress;
				triangles.maxVertex = mesh->getVertexCount();

				// Create acceleration structure geometry
				VkAccelerationStructureGeometryKHR geometry{};
				geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
				geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				geometry.geometry.triangles = triangles;

				// Create acceleration structure build range info
				VkAccelerationStructureBuildRangeInfoKHR offset{};
				offset.primitiveCount = maxPrimitiveCount;
				offset.primitiveOffset = 0;
				offset.firstVertex = 0;
				offset.transformOffset = 0;

				// Add new BottomLevelAccelerationStructureCreateInfo to list
				createInfos.emplace_back(BottomLevelAccelerationStructureCreateInfo{geometry, offset, &mesh->getSubmesh(submeshIndex).getBLAS()});
			}
		}

		return createInfos;
	}

	void buildBLAS(std::vector<BottomLevelAccelerationStructureCreateInfo>& input) {
		uint32_t blasCount = static_cast<uint32_t>(input.size());
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
			maxScratchSize = std::max(maxScratchSize, buildAS[i].sizeInfo.buildScratchSize);
		}

		// Allocate scratch buffers holding the temporary data of the acceleration structure builder.
		Buffer scratchBuffer;
		ResourceAllocator::createBuffer(maxScratchSize, scratchBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkDeviceAddress scratchAddress = scratchBuffer.getDeviceAddress();

		// Batch creation of BLAS to allow staying in restricted amount of memory.
		std::vector<uint32_t> indices;
		VkDeviceSize batchSize = 0;
		VkDeviceSize batchLimit = 256'000'000; // 256 MB
		for (uint32_t i = 0; i < blasCount; i++) {
			indices.push_back(i);
			batchSize += buildAS[i].sizeInfo.accelerationStructureSize;
			
			// Over the batch limit or the last BLAS element.
			if (batchSize >= batchLimit || i == blasCount - 1) {
				VkCommandBuffer cmdBuffer;
				EngineContext::createCommandBuffer(&cmdBuffer, 1);
				
				// Record command buffer.
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
				
				for (const auto& idx : indices) {
					// Allocate acceleration structure.
					ResourceAllocator::createAccelerationStructure(buildAS[idx].sizeInfo.accelerationStructureSize, *input[idx].pBLAS, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
					
					buildAS[idx].buildInfo.dstAccelerationStructure = input[idx].pBLAS->handle;
					buildAS[idx].buildInfo.scratchData.deviceAddress = scratchAddress;

					// Building the bottom-level-acceleration-structure
					vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildAS[idx].buildInfo, &buildAS[idx].rangeInfo);
					
					// Barrier to ensure that the scratch buffer is ready before building next acceleration structure.
					VkMemoryBarrier barrier{};
					barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
					barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
					barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
					vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
				}

				// End command buffer.
				VK_CHECK(vkEndCommandBuffer(cmdBuffer));

				// Submit command buffer and wait.
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmdBuffer;

				VK_CHECK(vkQueueSubmit(EngineContext::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
				VK_CHECK(vkQueueWaitIdle(EngineContext::getGraphicsQueue()));

				// Reset
				batchSize = 0;
				indices.clear();
			}
		}

		ResourceAllocator::destroyBuffer(scratchBuffer);
	}

	VkTransformMatrixKHR toTransformMatrixKHR(glm::mat4 matrix) {
		VkTransformMatrixKHR out;
		memcpy(&out.matrix, &glm::transpose(matrix), sizeof(VkTransformMatrixKHR));
		return out;
	}

	void buildTLAS(std::vector<Object>& objects, AccelerationStructure& tlas) {
		std::vector<VkAccelerationStructureInstanceKHR> instances;
		instances.reserve(objects.size());
		uint32_t nextInstanceIndex = 0;

		for (auto obj : objects) {
			for (uint32_t k = 0; k < obj.mesh->getSubmeshCount() && k < obj.materials.size(); k++) {
				VkAccelerationStructureInstanceKHR inst{};
				inst.transform = toTransformMatrixKHR(obj.transform);
				inst.instanceCustomIndex = nextInstanceIndex++;
				inst.accelerationStructureReference = obj.mesh->getSubmesh(k).getBLAS().getDeviceAddress();
				inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				inst.mask = 0xFF;
				inst.instanceShaderBindingTableRecordOffset = obj.shaderHitGroupOffset;
				instances.emplace_back(inst);
			}
		}

		uint32_t instCount = static_cast<uint32_t>(instances.size());

		// Create command buffer.
		VkCommandBuffer cmdBuffer;
		EngineContext::createCommandBuffer(&cmdBuffer, 1);

		// Record command buffer.
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

		// Create and stage a buffer holding the actual instance data for use by the AS builder.
		VkDeviceSize bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * instCount;
		Buffer instanceBuffer;
		Buffer instanceStageBuffer;
		ResourceAllocator::createAndStageBuffer(cmdBuffer, bufferSize, instances.data(), instanceStageBuffer, instanceBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
		VkDeviceAddress instanceBufferAddress = instanceBuffer.getDeviceAddress();

		// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build.
		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

		// Creating the TLAS.
		VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
		instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instancesData.data.deviceAddress = instanceBufferAddress;

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances = instancesData;

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
		sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(EngineContext::getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instCount, &sizeInfo);

		// Create TLAS buffer.
		ResourceAllocator::createAccelerationStructure(sizeInfo.accelerationStructureSize, tlas, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);

		// Build TLAS
		Buffer scratchBuffer;
		ResourceAllocator::createBuffer(sizeInfo.buildScratchSize, scratchBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		VkDeviceAddress scratchAddress = scratchBuffer.getDeviceAddress();

		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		buildInfo.dstAccelerationStructure = tlas.handle;
		buildInfo.scratchData.deviceAddress = scratchAddress;

		VkAccelerationStructureBuildRangeInfoKHR offsetInfo{};
		offsetInfo.primitiveCount = instCount;
		offsetInfo.primitiveOffset = 0;
		offsetInfo.firstVertex = 0;
		offsetInfo.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pOffsetInfo = &offsetInfo;

		vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildInfo, &pOffsetInfo);

		VkMemoryBarrier barrier2{};
		barrier2.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier2.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier2.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier2, 0, nullptr, 0, nullptr);

		// End command buffer.
		VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		// Submit command buffer and wait.
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VK_CHECK(vkQueueSubmit(EngineContext::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK(vkQueueWaitIdle(EngineContext::getGraphicsQueue()));

		// Cleanup.
		ResourceAllocator::destroyBuffer(scratchBuffer);
		ResourceAllocator::destroyBuffer(instanceStageBuffer);
		ResourceAllocator::destroyBuffer(instanceBuffer);
	}

	void Scene::buildAccelerationStructure(std::vector<Object>& objects, std::unordered_set<Mesh*>& meshes) {
		buildBLAS(fetchAllBottomLevelAccelerationStructureCreateInfo(meshes));
		buildTLAS(objects, tlas);
	}
}
