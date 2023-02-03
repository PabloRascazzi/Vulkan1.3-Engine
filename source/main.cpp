#include <engine_context.h>

#include <iostream>
#include <Vulkan/vulkan.h>

using namespace core;

int main() {
    // This is where most initialization for a program should be performed
    EngineContext::setup();

    // Print physical device name.
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties((VkPhysicalDevice)EngineContext::getPhysicalDevice(), &deviceProperties);
    std::cout << "Physical Device: " << deviceProperties.deviceName << std::endl;

    // Print queue indices.
    std::cout << "Graphics Queue: " << EngineContext::getGraphicsQueue() << std::endl;
    std::cout << "Present Queue:  " << EngineContext::getPresentQueue()  << std::endl;

    // Main loop
    while (EngineContext::update()) {

        // Update and render game here
    }

    // Clean up.
    EngineContext::cleanup();

    return 0;
}
