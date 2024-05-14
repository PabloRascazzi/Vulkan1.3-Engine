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
		~Renderer();

		virtual void cleanup() = 0;
		// TODO - virtual void render(Scene& scene) = 0;

	protected:
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

	};
}