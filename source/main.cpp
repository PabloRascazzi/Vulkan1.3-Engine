#include <engine_context.h>
#include <input.h>
#include <mesh.h>
#include <scene.h>
#include <descriptor_set.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>

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
    std::cout << "Max Bound Descriptor Sets: " << EngineContext::getDeviceProperties().limits.maxBoundDescriptorSets << std::endl;
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
    Mesh* mesh = new Mesh((float*)vertices.data(), static_cast<uint32_t>(vertices.size()), (uint32_t*)indices.data(), static_cast<uint32_t>(indices.size()));

    // Create Acceleration Structure.
    Scene* scene = new Scene();
    Object* obj = scene->addObject(mesh, glm::mat4(0), 0);
    scene->setup();

    // Create DescriptorSets.
    DescriptorSet* rtDescSet = new DescriptorSet();
    rtDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    rtDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    rtDescSet->create(EngineContext::getDevice());

    DescriptorSet* postDescSet = new DescriptorSet();
    postDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    postDescSet->create(EngineContext::getDevice());

    // Create Pipeline.
    StandardPipeline* pipeline = new StandardPipeline(EngineContext::getDevice(), "shader", EngineContext::getRenderPass(), EngineContext::getSwapChainExtent());
    RayTracingPipeline* RTpipeline = new RayTracingPipeline(EngineContext::getDevice(), std::vector<DescriptorSet*>{rtDescSet});
    PostPipeline* postPipeline = new PostPipeline(EngineContext::getDevice(), "postShader", std::vector<DescriptorSet*>{postDescSet}, EngineContext::getRenderPass(), EngineContext::getSwapChainExtent());

    // Fill DescriptorSets and create out images for ray-tracing render pass.
    std::vector<Image> outImages;
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        DescriptorSet::setCurrentFrame(i);

        Image outImage;
        EngineContext::createImage2D(EngineContext::getSwapChainExtent(), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, outImage.image, outImage.allocation);
        EngineContext::createImageView2D(outImage.image, VK_FORMAT_R32G32B32A32_SFLOAT, outImage.view);
        EngineContext::createSampler2D(outImage.sampler, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_TRUE);
        outImages.push_back(outImage);

        VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
        tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlasDescInfo.accelerationStructureCount = 1;
        tlasDescInfo.pAccelerationStructures = &scene->getTLAS().handle;
        rtDescSet->writeAccelerationStructureKHR(0, tlasDescInfo);

        VkDescriptorImageInfo rtImageDescInfo{};
        rtImageDescInfo.sampler = {};
        rtImageDescInfo.imageView = outImages[i].view;
        rtImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        rtDescSet->writeImage(1, rtImageDescInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        
        VkDescriptorImageInfo postImageDescInfo{};
        postImageDescInfo.sampler = outImages[i].sampler;
        postImageDescInfo.imageView = outImages[i].view;
        postImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        postDescSet->writeImage(0, postImageDescInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    }
    DescriptorSet::setCurrentFrame(0);
    rtDescSet->update();
    postDescSet->update();

    // Main loop
    while (EngineContext::update()) {
        if (Input::getKeyDown(INPUT_KEY_ESCAPE)) {
            EngineContext::exit();
        }

        // Update and render game here
        //EngineContext::rasterize((Pipeline&)*pipeline, *mesh);
        EngineContext::raytrace((Pipeline&)*RTpipeline, (Pipeline&)*postPipeline, *scene, outImages);

        // Reset Inputs.
        Input::reset();
    }

    // Wait for all GPU commands to finish.
    EngineContext::getDevice().waitIdle();

    // Clean up objects.
    delete mesh;
    delete pipeline;
    delete RTpipeline;
    delete postPipeline;
    for (auto image : outImages) 
        EngineContext::destroyImage(image);
    delete rtDescSet;
    delete postDescSet;
    delete scene;
    // Clean up engine.
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}
