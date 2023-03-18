#include <engine_context.h>
#include <input.h>
#include <mesh.h>
#include <scene.h>
#include <descriptor_set.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>
#include <resource_primitives.h>

#include <iostream>
#include <Vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace core;

glm::mat4 getPerspective(float vertical_fov, float aspect_ratio, float n, float f);

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
    std::cout << "Max Bound Descriptor Sets: " << EngineContext::getDeviceProperties().limits.maxBoundDescriptorSets << std::endl;
    std::cout << "Max Push Constant Size: " << EngineContext::getDeviceProperties().limits.maxPushConstantsSize << std::endl;
    std::cout << "Max Recursion Depth: " << EngineContext::getRayTracingProperties().maxRayRecursionDepth << std::endl;
    std::cout << "Max Geometry Count: " << EngineContext::getAccelerationStructureProperties().maxGeometryCount << std::endl;
    std::cout << "Max Instance Count: " << EngineContext::getAccelerationStructureProperties().maxInstanceCount << std::endl;

    // Create Mesh.
    Mesh* quad = ResourcePrimitives::createQuad(1.0f);
    Mesh* plane = ResourcePrimitives::createPlane(6, 1.0f);
    Mesh* cube = ResourcePrimitives::createCube(1.0f);

    // Create Scene.
    Scene* scene = new Scene();
    Object* quadObj  = scene->addObject(quad, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f)), 0);
    Object* planeObj = scene->addObject(plane, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -5.0f)), 0);
    Object* cubeObj = scene->addObject(cube, glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 2.0f, -2.5f)), 0);
    Camera camera{};
    camera.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)); // Opposite direction as world transforms.
    camera.viewInverse = glm::inverse(camera.view);
    camera.projection = getPerspective(60.0f, EngineContext::getWindow()->getAspectRatio(), 0.01f, 1000.0f);
    camera.projectionInverse = glm::inverse(camera.projection);
    scene->setup();

    // Create DescriptorSets.
    DescriptorSet* rtDescSet = new DescriptorSet();
    rtDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    rtDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    rtDescSet->create(EngineContext::getDevice());

    DescriptorSet* postDescSet = new DescriptorSet();
    postDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    postDescSet->create(EngineContext::getDevice());

    DescriptorSet* globalDescSet = new DescriptorSet();
    globalDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    globalDescSet->create(EngineContext::getDevice());

    // Create Camera matrices.
    struct CameraUniformBufferObject {
        glm::mat4 viewProj;    // view * projection
        glm::mat4 viewInverse; // inverse view matrix
        glm::mat4 projInverse; // inverse projection matrix
    };

    CameraUniformBufferObject cameraUBO{};
    cameraUBO.viewProj = camera.projection * camera.view;
    cameraUBO.viewInverse = camera.viewInverse;
    cameraUBO.projInverse = camera.projectionInverse;

    // Create Pipeline.
    StandardPipeline* pipeline = new StandardPipeline(EngineContext::getDevice(), "shader", EngineContext::getRenderPass(), EngineContext::getSwapChainExtent());
    RayTracingPipeline* RTpipeline = new RayTracingPipeline(EngineContext::getDevice(), std::vector<DescriptorSet*>{rtDescSet, globalDescSet});
    PostPipeline* postPipeline = new PostPipeline(EngineContext::getDevice(), "postShader", std::vector<DescriptorSet*>{postDescSet}, EngineContext::getRenderPass(), EngineContext::getSwapChainExtent());

    // Fill DescriptorSets and create out images for ray-tracing render pass.
    std::vector<Image> outImages;
    std::vector<Buffer> cameraBuffers;
    cameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        DescriptorSet::setCurrentFrame(i);

        // Upload TLAS uniform.
        VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
        tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlasDescInfo.accelerationStructureCount = 1;
        tlasDescInfo.pAccelerationStructures = &scene->getTLAS().handle;
        rtDescSet->writeAccelerationStructureKHR(0, tlasDescInfo);

        // Upload ray-tracing render pass output image uniform.
        Image outImage;
        ResourceAllocator::createImage2D(EngineContext::getSwapChainExtent(), VK_FORMAT_R32G32B32A32_SFLOAT, outImage, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
        ResourceAllocator::createImageView2D(VK_FORMAT_R32G32B32A32_SFLOAT, outImage);
        ResourceAllocator::createSampler2D(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_TRUE, outImage);
        outImages.push_back(outImage);

        VkDescriptorImageInfo rtImageDescInfo{};
        rtImageDescInfo.sampler = {};
        rtImageDescInfo.imageView = outImages[i].view;
        rtImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        rtDescSet->writeImage(1, rtImageDescInfo);
        
        VkDescriptorImageInfo postImageDescInfo{};
        postImageDescInfo.sampler = outImages[i].sampler;
        postImageDescInfo.imageView = outImages[i].view;
        postImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        postDescSet->writeImage(0, postImageDescInfo);

        // Upload camera matrices uniform.
        ResourceAllocator::createBuffer(sizeof(CameraUniformBufferObject), cameraBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        ResourceAllocator::mapDataToBuffer(cameraBuffers[i], sizeof(CameraUniformBufferObject), &cameraUBO);

        VkDescriptorBufferInfo camDescInfo{};
        camDescInfo.buffer = cameraBuffers[i].buffer;
        camDescInfo.offset = 0;
        camDescInfo.range = sizeof(CameraUniformBufferObject);
        globalDescSet->writeBuffer(0, camDescInfo);
    }
    DescriptorSet::setCurrentFrame(0);
    globalDescSet->update();
    rtDescSet->update();
    postDescSet->update();

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

        // Render using correct pipelines.
        if (raytrace) {
            EngineContext::raytrace((Pipeline&)*RTpipeline, (Pipeline&)*postPipeline, *scene, outImages);
        } else {
            EngineContext::rasterize((Pipeline&)*pipeline, camera, *scene);
        }

        // Reset Inputs.
        Input::reset();
    }

    // Wait for all GPU commands to finish.
    EngineContext::getDevice().waitIdle();

    // Clean up objects.
    delete quad;
    delete plane;
    delete cube;
    delete pipeline;
    delete RTpipeline;
    delete postPipeline;
    for (auto image : outImages) 
        ResourceAllocator::destroyImage(image);
    for (auto buffer : cameraBuffers)
        ResourceAllocator::destroyBuffer(buffer);
    delete rtDescSet;
    delete postDescSet;
    delete globalDescSet;
    delete scene;
    // Clean up engine.
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}

glm::mat4 getPerspective(float vertical_fov, float aspect_ratio, float n, float f) {
    float fov_rad = vertical_fov * 2.0f * static_cast<float>(M_PI) / 360.0f;
    float focal_length = 1.0f / std::tan(fov_rad / 2.0f);

    float x  =  focal_length / aspect_ratio;
    float y  = -focal_length;
    float A = f / (n - f);
    float B = (n * f) / (n - f);

    glm::mat4 projection({
        x,    0.0f,  0.0f,  0.0f,
        0.0f,    y,  0.0f,  0.0f,
        0.0f, 0.0f,     A, -1.0f,
        0.0f, 0.0f,     B,  0.0f,
    });
    return projection;
}
