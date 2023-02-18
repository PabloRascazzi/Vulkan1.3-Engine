#include <engine_context.h>
#include <input.h>
#include <mesh.h>
#include <scene.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>

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
    std::cout << "Present Queue:  " << EngineContext::getPresentQueue() << std::endl;

    // Print Device Properties.
    std::cout << "Max Push Constant Size: " << EngineContext::getDeviceProperties().limits.maxPushConstantsSize << std::endl;
    std::cout << "Max Recursion Depth: " << EngineContext::getRayTracingProperties().maxRayRecursionDepth << std::endl;
    std::cout << "Max Geometry Count: " << EngineContext::getAccelerationStructureProperties().maxGeometryCount << std::endl;
    std::cout << "Max Instance Count: " << EngineContext::getAccelerationStructureProperties().maxInstanceCount << std::endl;

    // Create Mesh.
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    const std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0,
    };
    Mesh* mesh = new Mesh((float*)vertices.data(), vertices.size(), (uint32_t*)indices.data(), indices.size());

    // Create Acceleration Structure.
    Scene scene = Scene();
    Object* obj = scene.addObject(mesh, glm::mat4(0), 0);
    scene.setup();

    // Create Pipeline.
    StandardPipeline* pipeline = new StandardPipeline(EngineContext::getDevice(), EngineContext::getRenderPass(), EngineContext::getSwapChainExtent());
    RayTracingPipeline* RTpipeline = new RayTracingPipeline(EngineContext::getDevice());

    // Main loop
    while (EngineContext::update()) {
        if (Input::getKeyDown(INPUT_KEY_ESCAPE)) {
            EngineContext::exit();
        }

        // Update and render game here
        EngineContext::rasterize((Pipeline&)*pipeline, *mesh);

        // Reset Inputs.
        Input::reset();
    }

    // Wait for all GPU commands to finish.
    EngineContext::getDevice().waitIdle();

    // Clean up objects.
    delete mesh;
    delete pipeline;
    delete RTpipeline;
    // Clean up engine.
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}
