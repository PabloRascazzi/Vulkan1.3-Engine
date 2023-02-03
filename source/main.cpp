#include <engine_context.h>
#include <input.h>


#include <iostream>
#include <Vulkan/vulkan.h>

using namespace core;

int main() {
    Input::setup();

    // This is where most initialization for a program should be performed
    EngineContext::getWindow()->setKeyCallback(Input::keyCallback);
    EngineContext::getWindow()->setMouseButtonCallback(Input::mouseButtonCallback);
    EngineContext::getWindow()->setMouseMotionCallback(Input::mouseMotionCallback);
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
        if (Input::getKeyDown(INPUT_KEY_ESCAPE)) {
            EngineContext::exit();
        }

        // Update and render game here

        // Reset Inputs.
        Input::reset();
    }

    // Clean up engine.
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}
