#include <engine_globals.h>
#include <engine_context.h>
#include <renderer/standard_renderer.h>
#include <renderer/pathtraced_renderer.h>
#include <input.h>
#include <mesh.h>
#include <texture.h>
#include <camera.h>
#include <scene.h>
#include <descriptor_set.h>
#include <pipeline/standard_pipeline.h>
#include <pipeline/raytracing_pipeline.h>
#include <pipeline/post_pipeline.h>
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
    Object* demoCube = scene->addObject(cube, std::vector{cubeMat}, glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f, -0.5f, -5.0f)), 0);
    Object* floor    = scene->addObject(plane, std::vector{floorMat}, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -5.0f)), 0);
    Object* anvilObj = scene->addObject(anvil, std::vector{anvilMat}, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f,  -2.0f)), 0);
    Camera* camera   = scene->addCamera(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f)), 60.0f, EngineContext::getWindow().getAspectRatio());
    // Setup Scene.
    scene->setup();

    // Create DescriptorSets.
    DescriptorSet* rtDescSet = new DescriptorSet();
    rtDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    rtDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    rtDescSet->addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    rtDescSet->create(EngineContext::getDevice());

    DescriptorSet* postDescSet = new DescriptorSet();
    postDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    postDescSet->create(EngineContext::getDevice());

    DescriptorSet* globalDescSet = new DescriptorSet();
    globalDescSet->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    globalDescSet->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(scene->getTextures().size()), VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    globalDescSet->create(EngineContext::getDevice());

    // Create Camera matrices.
    struct CameraUniformBufferObject {
        glm::mat4 viewProj;    // view * projection
        glm::mat4 viewInverse; // inverse view matrix
        glm::mat4 projInverse; // inverse projection matrix
        glm::vec3 position; // camera's world position
    };

    CameraUniformBufferObject cameraUBO{};
    cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
    cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
    cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();
    
    // Create Renderer.
    StandardRenderer* renderer = new StandardRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue());
    PathTracedRenderer* ptRenderer = new PathTracedRenderer(EngineContext::getDevice(), EngineContext::getGraphicsQueue(), EngineContext::getPresentQueue());

    // Create Pipeline.
    StandardPipeline* pipeline = new StandardPipeline(EngineContext::getDevice(), "shader", std::vector<DescriptorSet*>{globalDescSet}, renderer->getRenderPass(), Renderer::getSwapChainExtent());
    RayTracingPipeline* RTpipeline = new RayTracingPipeline(EngineContext::getDevice(), std::vector<DescriptorSet*>{rtDescSet, globalDescSet});
    PostPipeline* postPipeline = new PostPipeline(EngineContext::getDevice(), "postShader", std::vector<DescriptorSet*>{postDescSet}, ptRenderer->getRenderPass(), Renderer::getSwapChainExtent());

    // Fill DescriptorSets and create out images for ray-tracing render pass.
    std::vector<Texture*> outTextures;
    std::vector<Buffer> cameraBuffers;
    cameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkDescriptorImageInfo* globalTextureDescInfos = new VkDescriptorImageInfo[scene->getTextures().size()*MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        DescriptorSet::setCurrentFrame(i);

        // Upload TLAS uniform.
        VkWriteDescriptorSetAccelerationStructureKHR tlasDescInfo{};
        tlasDescInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlasDescInfo.accelerationStructureCount = 1;
        tlasDescInfo.pAccelerationStructures = &scene->getTLAS().handle;
        rtDescSet->writeAccelerationStructureKHR(0, tlasDescInfo);

        // Upload ray-tracing render pass output image uniform.
        VkExtent2D outExtent = VkExtent2D{Renderer::getSwapChainExtent()};
        Texture* outTexture = new Texture(outExtent, nullptr, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_FALSE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        EngineContext::transitionImageLayout(outTexture->getImage().image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        outTextures.push_back(outTexture);

        VkDescriptorImageInfo rtImageDescInfo{};
        rtImageDescInfo.sampler = {};
        rtImageDescInfo.imageView = outTextures[i]->getImageView();
        rtImageDescInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        rtDescSet->writeImage(1, rtImageDescInfo);
        
        VkDescriptorImageInfo postImageDescInfo{};
        postImageDescInfo.sampler = outTextures[i]->getSampler();
        postImageDescInfo.imageView = outTextures[i]->getImageView();
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

        // Upload objects description buffer.
        VkDescriptorBufferInfo objDescInfo{};
        objDescInfo.buffer = scene->getObjDescriptions().buffer;
        objDescInfo.offset = 0;
        objDescInfo.range = VK_WHOLE_SIZE;
        rtDescSet->writeBuffer(2, objDescInfo);

        // Upload texture samplers.
        for (uint32_t texIndex = 0; texIndex < scene->getTextures().size(); texIndex++) {
            size_t texDescIndex = texIndex + (scene->getTextures().size() * i);
            globalTextureDescInfos[texDescIndex].sampler = scene->getTextures()[texIndex]->getSampler();
            globalTextureDescInfos[texDescIndex].imageView = scene->getTextures()[texIndex]->getImageView();
            globalTextureDescInfos[texDescIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            globalDescSet->writeImage(1, globalTextureDescInfos[texDescIndex], texIndex);
        }
    }
    DescriptorSet::setCurrentFrame(0);
    globalDescSet->update();
    rtDescSet->update();
    postDescSet->update();
    delete[] globalTextureDescInfos;

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

        // Update main Camera uniform buffer.
        cameraUBO.viewProj = scene->getMainCamera().getProjectionMatrix() * scene->getMainCamera().getViewMatrix();
        cameraUBO.viewInverse = scene->getMainCamera().getViewInverseMatrix();
        cameraUBO.projInverse = scene->getMainCamera().getProjectionInverseMatrix();

        ResourceAllocator::mapDataToBuffer(cameraBuffers[Renderer::getCurrentFrame()], sizeof(CameraUniformBufferObject), &cameraUBO);

        VkDescriptorBufferInfo camDescInfo{};
        camDescInfo.buffer = cameraBuffers[Renderer::getCurrentFrame()].buffer;
        camDescInfo.offset = 0;
        camDescInfo.range = sizeof(CameraUniformBufferObject);
        globalDescSet->writeBuffer(0, camDescInfo);

        // Render using correct pipelines.
        if (raytrace) {
            ptRenderer->render((Pipeline&)*RTpipeline, (Pipeline&)*postPipeline, *scene);
        } else {
            renderer->render((Pipeline&)*pipeline, *scene);
        }

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
//    delete pipeline;
//    delete renderer;
//    delete ptRenderer;
//    delete RTpipeline;
//    delete postPipeline;
//    for (auto outTexture : outTextures)
//        delete outTexture;
//    for (auto buffer : cameraBuffers)
//        ResourceAllocator::destroyBuffer(buffer);
//    delete rtDescSet;
//    delete postDescSet;
//    delete globalDescSet;
    delete scene;
    // Clean up engine.
//    Renderer::destroySwapchain();
    EngineContext::cleanup();
    Input::cleanup();

    return 0;
}
