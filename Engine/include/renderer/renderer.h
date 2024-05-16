#pragma once
#include <engine_globals.h>
#include <pipeline/pipeline.h>
#include <scene.h>

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {

	class Renderer {
	public:
		Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue);
		~Renderer() = default;

		virtual void render(const uint32_t currentFrame, Scene& scene) = 0;

	protected:
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

	};
}