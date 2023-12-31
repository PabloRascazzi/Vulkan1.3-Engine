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
    Input::setup();

    // This is where most initialization for a program should be performed
    EngineContext::getWindow().setKeyCallback(Input::keyCallback);
    EngineContext::getWindow().setMouseButtonCallback(Input::mouseButtonCallback);
    EngineContext::getWindow().setMouseMotionCallback(Input::mouseMotionCallback);
    EngineContext::setup();

    // Print physical device name.
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties((VkPhysicalDevice)EngineContext::getPhysicalDevice(), &deviceProperties);
    std::cout << "Physical Device: " << deviceProperties.deviceName << std::endl;

    // Print queue indices.
    std::cout << "Graphics Queue: " << EngineContext::getGraphicsQueue() << std::endl;
    std::cout << "Present Queue:  " << EngineContext::getPresentQueue() << std::endl;

    // Print Device Properties.
    std::cout << "Max Bound Descriptor Sets: " << EngineContext::getPhysicalDeviceProperties().deviceProperties.limits.maxBoundDescriptorSets << std::endl;
    std::cout << "Max Push Constant Size: " << EngineContext::getPhysicalDeviceProperties().deviceProperties.limits.maxPushConstantsSize << std::endl;
    std::cout << "Max Recursion Depth: " << EngineContext::getPhysicalDeviceProperties().raytracingProperties.maxRayRecursionDepth << std::endl;
    std::cout << "Max Geometry Count: " << EngineContext::getPhysicalDeviceProperties().accelStructProperties.maxGeometryCount << std::endl;
    std::cout << "Max Instance Count: " << EngineContext::getPhysicalDeviceProperties().accelStructProperties.maxInstanceCount << std::endl;

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
    Scene* scene = new Scene();
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
    Camera* camera   = scene->addCamera(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f)), 60.0f, EngineContext::getWindow().getAspectRatio());
    // Setup Scene.
    scene->setup();

    // Select scene to render and setup renderer.
    EngineRenderer::setup(scene);

    // Main loop
    bool raytrace = true;
    while (EngineContext::update()) {
        if (Input::getKeyDown(INPUT_KEY_ESCAPE)) {
            EngineContext::exit();
        }

        // Update and render game starting here.
        
        // Toggle raytracing.
        if (Input::getKey(INPUT_KEY_1)) {
            raytrace = true;
        } else if (Input::getKey(INPUT_KEY_2)) {
            raytrace = false;
        }

        // Update scene.
        scene->update();

        // Render scene.
        EngineRenderer::render(raytrace);

        // Reset Inputs.
        Input::reset();
    }

    // Wait for all GPU commands to finish.
    EngineContext::getDevice().waitIdle();

    // Clean up objects.
    delete riverDirtNormal;
    delete riverDirtDiffuse;
    delete testTex;
    delete anvil;
    delete quad;
    delete plane;
    delete cube;
    delete scene;
    EngineRenderer::cleanup();
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}
