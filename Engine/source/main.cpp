#include <engine_globals.h>
#include <engine_context.h>
#include <engine_renderer.h>
#include <input.h>
#include <mesh.h>
#include <texture.h>
#include <camera.h>
#include <scene.h>
#include <resource_primitives.h>
#include <file_reader.h>

#include <iostream>
#include <Vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace core;

int main() {
    EngineContext& context = EngineContext::GetInstance();
    Input::setup();

    // This is where most initialization for a program should be performed
    context.getWindow().setKeyCallback(Input::keyCallback);
    context.getWindow().setMouseButtonCallback(Input::mouseButtonCallback);
    context.getWindow().setMouseMotionCallback(Input::mouseMotionCallback);

    // Print physical device name.
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties((VkPhysicalDevice)context.getPhysicalDevice(), &deviceProperties);
    std::cout << "Physical Device: " << deviceProperties.deviceName << std::endl;

    // Print queue indices.
    std::cout << "Graphics Queue: " << context.getGraphicsQueue() << std::endl;
    std::cout << "Present Queue:  " << context.getPresentQueue() << std::endl;

    // Print Device Properties.
    std::cout << "Max Bound Descriptor Sets: " << context.getPhysicalDeviceProperties().deviceProperties.limits.maxBoundDescriptorSets << std::endl;
    std::cout << "Max Push Constant Size: " << context.getPhysicalDeviceProperties().deviceProperties.limits.maxPushConstantsSize << std::endl;
    std::cout << "Max Recursion Depth: " << context.getPhysicalDeviceProperties().raytracingProperties.maxRayRecursionDepth << std::endl;
    std::cout << "Max Geometry Count: " << context.getPhysicalDeviceProperties().accelStructProperties.maxGeometryCount << std::endl;
    std::cout << "Max Instance Count: " << context.getPhysicalDeviceProperties().accelStructProperties.maxInstanceCount << std::endl;
    std::cout << "Min Uniform Buffer Offset Alignment: " << context.getPhysicalDeviceProperties().deviceProperties.limits.minUniformBufferOffsetAlignment << std::endl;

    // Create Meshes.
    Mesh* anvil = FileReader::readMeshFile("anvil");
    Mesh* quad = ResourcePrimitives::createQuad(2.0f);
    Mesh* plane = ResourcePrimitives::createPlane(6, 2.0f);
    Mesh* cube = ResourcePrimitives::createCube(1.0f);

    // Create Textures.
    Texture* testTex = FileReader::readImageFile("test.png", COLOR_SPACE_SRGB);
    Texture* riverDirtDiffuse = FileReader::readImageFile("RiverDirt_Diffuse_512.png", COLOR_SPACE_SRGB);
    Texture* riverDirtNormal = FileReader::readImageFile("RiverDirt_Normals_512.png", COLOR_SPACE_LINEAR);

    // Create Scene.
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    // Create Materials.
    Material* cubeMat =   scene->addMaterial(riverDirtDiffuse, glm::vec3(1.0f, 0.0f, 0.0f), nullptr, 0.0f, 0.5f, riverDirtNormal, glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Material* mirrorMat = scene->addMaterial(testTex, glm::vec3(0.0f, 1.0f, 0.0f), nullptr, 0.0f, 0.5f, nullptr, glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Material* floorMat =  scene->addMaterial(testTex, glm::vec3(0.0f, 0.0f, 1.0f), nullptr, 0.0f, 0.5f, nullptr, glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Material* anvilMat =  scene->addMaterial(riverDirtDiffuse, glm::vec3(1.0f, 0.0f, 1.0f), nullptr, 0.0f, 0.5f, riverDirtNormal, glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    // Create Objects.
    glm::mat4 mirrorRotation = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    Object* mirror1  = scene->addObject(quad, std::vector{mirrorMat}, glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, -7.5f))* (glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * mirrorRotation), 0);
    Object* mirror2  = scene->addObject(quad, std::vector{mirrorMat}, glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f,  0.0f, -2.5f)) * (glm::rotate(glm::mat4(1.0f), glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * mirrorRotation), 0);
    Object* demoCube = scene->addObject(cube, std::vector{ cubeMat }, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -5.0f)), 0);
    Object* floor    = scene->addObject(plane, std::vector{ floorMat }, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -5.0f)), 0);
    Object* anvilObj = scene->addObject(anvil, std::vector{ anvilMat }, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -2.0f)), 0);
    Camera* camera   = scene->addCamera(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f)), 45.0f, glm::uvec2(context.getWindow().getWidth(), context.getWindow().getHeight()));
    // Setup Scene.
    scene->setup();

    // Select scene to render and setup renderer.
    EngineRenderer renderer = EngineRenderer(scene);

    // Main loop
    uint8_t rendermode = 0;
    while (context.update()) {
        if (Input::getKeyDown(INPUT_KEY_ESCAPE)) {
            context.exit();
        }

        // Update and render game starting here.
        
        // Toggle raytracing.
        if (Input::getKey(INPUT_KEY_1)) {
            rendermode = 0;
        } else if (Input::getKey(INPUT_KEY_2)) {
            rendermode = 1;
        }else if (Input::getKey(INPUT_KEY_3)) {
            rendermode = 2;
        }

        // Update scene.
        scene->update();

        // Render scene.
        renderer.Render(rendermode);

        // Reset Inputs.
        Input::reset();
    }

    // Wait for all GPU commands to finish.
    context.getDevice().waitIdle();

    // Clean up objects.
    delete riverDirtNormal;
    delete riverDirtDiffuse;
    delete testTex;
    delete anvil;
    delete quad;
    delete plane;
    delete cube;
    Input::cleanup();
}
