#include <renderer/renderer.h>
#include <engine_context.h>
#include <engine_globals.h>

namespace core {

	Renderer::Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue) {
		this->device = device;
		this->graphicsQueue = graphicsQueue;
		this->presentQueue = presentQueue;
	}

	Renderer::~Renderer() {

	}

}