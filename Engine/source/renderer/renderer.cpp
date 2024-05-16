#include <renderer/renderer.h>
#include <engine_context.h>
#include <engine_globals.h>

namespace core {

	Renderer::Renderer(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue) : 
		device(device), graphicsQueue(graphicsQueue), presentQueue(presentQueue) {}
}