#include "utils/common.h"
#include "VulkanGraphicsApp.h"
#include "data/VertexInput.h"
#include <glm/glm.hpp>
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>

    
void VulkanGraphicsApp::init(){
    initHandledUniforms();
    initRenderPipeline();
    initFramebuffers();
    initCommands();
    initSync();
}

const VkExtent2D& VulkanGraphicsApp::getFramebufferSize() const{
    return(mViewportExtent);
}

void VulkanGraphicsApp::setVertexInput(
    const VkVertexInputBindingDescription& aBindingDescription,
    const std::vector<VkVertexInputAttributeDescription>& aAttributeDescriptions
){
    mBindingDescription = aBindingDescription;
    mAttributeDescriptions = aAttributeDescriptions;
    if(mVertexInputsHaveBeenSet){
        //TODO: Verify this works 
        resetRenderSetup();
    }
    mVertexInputsHaveBeenSet = true;
}

void VulkanGraphicsApp::setVertexBuffer(const VkBuffer& aBuffer, size_t aVertexCount){
    bool needsReset = mVertexBuffer != VK_NULL_HANDLE && (mVertexBuffer != aBuffer || mVertexCount != aVertexCount); 
    mVertexBuffer = aBuffer;
    mVertexCount = aVertexCount;
    if(needsReset) resetRenderSetup(); // TODO: Verify 
}

void VulkanGraphicsApp::setVertexShader(const std::string& aShaderName, const VkShaderModule& aShaderModule){
    if(aShaderName.empty() || aShaderModule == VK_NULL_HANDLE){
        throw std::runtime_error("VulkanGraphicsApp::setVertexShader() Error: Arguments must be a non-empty string and valid shader module!");
    }
    mShaderModules[aShaderName] = aShaderModule;
    mVertexKey = aShaderName;
    if(mVertexKey == mFragmentKey){
        throw std::runtime_error("Error: Keys/Names for the vertex and fragment shader cannot be the same!");
    }
}

void VulkanGraphicsApp::setFragmentShader(const std::string& aShaderName, const VkShaderModule& aShaderModule){
    if(aShaderName.empty() || aShaderModule == VK_NULL_HANDLE){
        throw std::runtime_error("VulkanGraphicsApp::setFragmentShader() Error: Arguments must be a non-empty string and valid shader module!");
    }
    mShaderModules[aShaderName] = aShaderModule;
    mFragmentKey = aShaderName;
    if(mVertexKey == mFragmentKey){
        throw std::runtime_error("Error: Keys/Names for the vertex and fragment shader cannot be the same!");
    }
}

void VulkanGraphicsApp::addUniformHandler(uint32_t aBindingPoint, UniformHandlerPtr aHandlerPtr, VkShaderStageFlags aStages){
    if(aHandlerPtr == nullptr){
        std::cerr << "Ignoring attempt to add nullptr as uniform handler!" << std::endl;
        return;
    }
    auto findExisting = mUniformHandlers.find(aBindingPoint);
    if(findExisting != mUniformHandlers.end()){
        if(findExisting->second != nullptr) findExisting->second->free();
        findExisting->second = aHandlerPtr;
    }else{
        mUniformHandlers.emplace(aBindingPoint, aHandlerPtr);
    }

    if(mRenderPipeline.isValid())
        resetRenderSetup();
}

UniformHandlerPtr VulkanGraphicsApp::removeUniformHandler(uint32_t aBindingPoint){
    auto finder = mUniformHandlers.find(aBindingPoint);
    if(finder != mUniformHandlers.end()){
        UniformHandlerPtr removed = finder->second;
        mUniformHandlers.erase(finder);
        return(removed);
    }
    return(nullptr);
}

void VulkanGraphicsApp::resetRenderSetup(){
    vkDeviceWaitIdle(mLogicalDevice.handle());

    cleanupSwapchainDependents();
    VulkanSetupBaseApp::cleanupSwapchain();

    VulkanSetupBaseApp::initSwapchain();
    initHandledUniforms();
    initRenderPipeline();
    initFramebuffers();
    initCommands();
    initSync();

    sWindowFlags[mWindow].resized = false;
}

void VulkanGraphicsApp::render(){
    uint32_t targetImageIndex = 0;
    size_t syncObjectIndex = mFrameNumber % IN_FLIGHT_FRAME_LIMIT;

    vkWaitForFences(mLogicalDevice.handle(), 1, &mInFlightFences[syncObjectIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(mLogicalDevice.handle(),
        mSwapchainBundle.swapchain, std::numeric_limits<uint64_t>::max(),
        mImageAvailableSemaphores[syncObjectIndex], VK_NULL_HANDLE, &targetImageIndex
    );
    if(result == VK_ERROR_OUT_OF_DATE_KHR || sWindowFlags[mWindow].resized){
        resetRenderSetup();
        render();
        return;
    }else if(result == VK_SUBOPTIMAL_KHR){
        std::cerr << "Warning! Swapchain suboptimal" << std::endl;
    }else if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to get next image in swapchain!");
    }

    const static VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
        1, &mImageAvailableSemaphores[syncObjectIndex], &waitStages,
        1, &mCommandBuffers[targetImageIndex],
        1, &mRenderFinishSemaphores[syncObjectIndex]
    };

    vkResetFences(mLogicalDevice.handle(), 1, &mInFlightFences[syncObjectIndex]);

    if(vkQueueSubmit(mLogicalDevice.getGraphicsQueue(), 1, &submitInfo, mInFlightFences[syncObjectIndex]) != VK_SUCCESS){
        throw std::runtime_error("Submit to graphics queue failed!");
    }

    VkPresentInfoKHR presentInfo = {
        /*sType = */ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        /*pNext = */ nullptr,
        /*waitSemaphoreCount = */ 1,
        /*pWaitSemaphores = */ &mRenderFinishSemaphores[syncObjectIndex],
        /*swapchainCount = */ 1,
        /*pSwapchains = */ &mSwapchainBundle.swapchain,
        /*pImageIndices = */ &targetImageIndex,
        /*pResults = */ nullptr
    };

    vkQueuePresentKHR(mLogicalDevice.getPresentationQueue(), &presentInfo);
}

void VulkanGraphicsApp::initRenderPipeline(){
    if(!mVertexInputsHaveBeenSet){
        throw std::runtime_error("Error! Render pipeline cannot be created before vertex input information has been set via 'setVertexInput()'");
    }else if(mVertexKey.empty()){
        throw std::runtime_error("Error! No vertex shader has been set! A vertex shader must be set using setVertexShader()!");
    }else if(mFragmentKey.empty()){
        throw std::runtime_error("Error! No fragment shader has been set! A vertex shader must be set using setFragmentShader()!");
    }

    vkutils::GraphicsPipelineConstructionSet& ctorSet =  mRenderPipeline.setupConstructionSet(mLogicalDevice.handle(), &mSwapchainBundle);
    vkutils::BasicVulkanRenderPipeline::prepareFixedStages(ctorSet);

    VkShaderModule vertShader = VK_NULL_HANDLE;
    VkShaderModule fragShader = VK_NULL_HANDLE;
    auto findVert = mShaderModules.find(mVertexKey);
    auto findFrag = mShaderModules.find(mFragmentKey);
    if(findVert == mShaderModules.end()){
        throw std::runtime_error("Error: Vertex shader name '" + mVertexKey + "' did not map to a valid shader module");
    }else{
        vertShader = findVert->second;
    }
    if(findFrag == mShaderModules.end()){
        std::cerr << "Error: Fragment shader name '" + mFragmentKey + "' did not map to a valid shader module. Using fallback..." << std::endl;
        findFrag = mShaderModules.find("fallback.frag");
        if(findFrag != mShaderModules.end()){
            fragShader = findFrag->second;
        }else{
            fragShader = vkutils::load_shader_module(mLogicalDevice.handle(), STRIFY(SHADER_DIR) "/fallback.frag.spv");
            mShaderModules["fallback.frag"] = fragShader;
        }
    }else{
        fragShader = findFrag->second;
    }
    
    VkPipelineShaderStageCreateInfo vertStageInfo;{
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.pNext = nullptr;
        vertStageInfo.flags = 0;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = vertShader;
        vertStageInfo.pName = "main";
        vertStageInfo.pSpecializationInfo = nullptr;
    }
    VkPipelineShaderStageCreateInfo fragStageInfo;{
        fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageInfo.pNext = nullptr;
        fragStageInfo.flags = 0;
        fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageInfo.module = fragShader;
        fragStageInfo.pName = "main";
        fragStageInfo.pSpecializationInfo = nullptr;
    }
    ctorSet.mProgrammableStages.emplace_back(vertStageInfo);
    ctorSet.mProgrammableStages.emplace_back(fragStageInfo);

    ctorSet.mVtxInputInfo.pVertexBindingDescriptions = &mBindingDescription;
    ctorSet.mVtxInputInfo.vertexBindingDescriptionCount = 1U;
    ctorSet.mVtxInputInfo.pVertexAttributeDescriptions = mAttributeDescriptions.data();
    ctorSet.mVtxInputInfo.vertexAttributeDescriptionCount = mAttributeDescriptions.size();

    ctorSet.mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    ctorSet.mPipelineLayoutInfo.pNext = 0;
    ctorSet.mPipelineLayoutInfo.flags = 0;
    ctorSet.mPipelineLayoutInfo.setLayoutCount = mUniformDescriptorLayouts.size();
    ctorSet.mPipelineLayoutInfo.pSetLayouts = mUniformDescriptorLayouts.data();
    ctorSet.mPipelineLayoutInfo.pushConstantRangeCount = 0;
    ctorSet.mPipelineLayoutInfo.pPushConstantRanges = nullptr;

    vkutils::BasicVulkanRenderPipeline::prepareViewport(ctorSet);
    vkutils::BasicVulkanRenderPipeline::prepareRenderPass(ctorSet);
    mRenderPipeline.build(ctorSet);
}

void VulkanGraphicsApp::initCommands(){
    VkCommandPoolCreateInfo poolInfo;{
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = nullptr;
        poolInfo.flags = 0;
        poolInfo.queueFamilyIndex = *mPhysDevice.mGraphicsIdx;
    }

    if(mCommandPool == VK_NULL_HANDLE){
        if(vkCreateCommandPool(mLogicalDevice.handle(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS){
            throw std::runtime_error("Failed to create command pool for graphics queue!");
        }
    }

    mCommandBuffers.resize(mSwapchainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo;{
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.commandBufferCount = mCommandBuffers.size();
        allocInfo.commandPool = mCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }
    if(vkAllocateCommandBuffers(mLogicalDevice.handle(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for(size_t i = 0; i < mCommandBuffers.size(); ++i){
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0 , nullptr};
        if(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("Failed to begine command recording!");
        }

        static const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo renderBegin;{
            renderBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderBegin.pNext = nullptr;
            renderBegin.renderPass = mRenderPipeline.getRenderpass();
            renderBegin.framebuffer = mSwapchainFramebuffers[i];
            renderBegin.renderArea = {{0,0}, mSwapchainBundle.extent};
            renderBegin.clearValueCount = 1;
            renderBegin.pClearValues = &clearColor;
        }

        vkCmdBeginRenderPass(mCommandBuffers[i], &renderBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipeline.getPipeline());
        vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, &mVertexBuffer, std::array<VkDeviceSize, 1>{0}.data());

        // TODO: fix offsets for multiple uniforms
        vkCmdBindDescriptorSets(
            mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipeline.getLayout(),
            0, mUniformDescriptorSets[i].size(), mUniformDescriptorSets[i].data(),
            0, nullptr
        );

        vkCmdDraw(mCommandBuffers[i], mVertexCount, 1, 0, 0);
        vkCmdEndRenderPass(mCommandBuffers[i]);

        if(vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to end command buffer " + std::to_string(i));
        }
    }
}

void VulkanGraphicsApp::initFramebuffers(){
    mSwapchainFramebuffers.resize(mSwapchainBundle.views.size());
    for(size_t i = 0; i < mSwapchainBundle.views.size(); ++i){
        VkFramebufferCreateInfo framebufferInfo;{
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0;
            framebufferInfo.renderPass = mRenderPipeline.getRenderpass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &mSwapchainBundle.views[i];
            framebufferInfo.width = mSwapchainBundle.extent.width;
            framebufferInfo.height = mSwapchainBundle.extent.height;
            framebufferInfo.layers = 1;
        }

        if(vkCreateFramebuffer(mLogicalDevice.handle(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create swap chain framebuffer " + std::to_string(i));
        }
    }
}

void VulkanGraphicsApp::initSync(){
    mImageAvailableSemaphores.resize(IN_FLIGHT_FRAME_LIMIT);
    mRenderFinishSemaphores.resize(IN_FLIGHT_FRAME_LIMIT);
    mInFlightFences.resize(IN_FLIGHT_FRAME_LIMIT);

    VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    VkSemaphoreCreateInfo semaphoreCreate = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};

    bool failure = false;
    for(size_t i = 0 ; i < IN_FLIGHT_FRAME_LIMIT; ++i){
        failure |= vkCreateSemaphore(mLogicalDevice.handle(), &semaphoreCreate, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS;
        failure |= vkCreateSemaphore(mLogicalDevice.handle(), &semaphoreCreate, nullptr, &mRenderFinishSemaphores[i]) != VK_SUCCESS;
        failure |= vkCreateFence(mLogicalDevice.handle(), &fenceInfo, nullptr, &mInFlightFences[i]);
    }
    if(failure){
        throw std::runtime_error("Failed to create semaphores!");
    }

}

void VulkanGraphicsApp::cleanupSwapchainDependents(){
    vkDestroyDescriptorPool(mLogicalDevice, mUniformDescriptorPool, nullptr);

    for(size_t i = 0; i < IN_FLIGHT_FRAME_LIMIT; ++i){
        vkDestroySemaphore(mLogicalDevice.handle(), mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mLogicalDevice.handle(), mRenderFinishSemaphores[i], nullptr);
        vkDestroyFence(mLogicalDevice.handle(), mInFlightFences[i], nullptr);
    }

    vkFreeCommandBuffers(mLogicalDevice.handle(), mCommandPool, mCommandBuffers.size(), mCommandBuffers.data());
    
    for(const VkFramebuffer& fb : mSwapchainFramebuffers){
        vkDestroyFramebuffer(mLogicalDevice.handle(), fb, nullptr);
    }

    mRenderPipeline.destroy();
}

void VulkanGraphicsApp::cleanup(){
    for(std::pair<const std::string, VkShaderModule>& module : mShaderModules){
        vkDestroyShaderModule(mLogicalDevice.handle(), module.second, nullptr);
    }

    cleanupSwapchainDependents();

    mUniformHandlers.freeAllAndClear();

    vkDestroyCommandPool(mLogicalDevice.handle(), mCommandPool, nullptr);

    VulkanSetupBaseApp::cleanup();
}

void VulkanGraphicsApp::initHandledUniforms() {
    VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    for(std::pair<uint32_t, UniformHandlerPtr> uniform : mUniformHandlers){
        if(uniform.second->isInited()){
            uniform.second->free();
        }
        uniform.second->init(mSwapchainBundle.views.size(), uniform.first, {mLogicalDevice, mPhysDevice}, stages);
    }

    initUniformDescriptorPool();
    initUniformDescriptorSets();
}
    
void VulkanGraphicsApp::initUniformDescriptorPool() {
    VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = mUniformHandlers.getTotalUniformBufferCount();

    VkDescriptorPoolCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.maxSets = mUniformHandlers.getTotalUniformBufferCount();
        createInfo.poolSizeCount = 1;
        createInfo.pPoolSizes = &poolSize;
    }

    if(vkCreateDescriptorPool(mLogicalDevice, &createInfo, nullptr, &mUniformDescriptorPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create uniform descriptor pool");
    }
}
    
void VulkanGraphicsApp::initUniformDescriptorSets() {
    uint32_t totalBufferCount = mUniformHandlers.getTotalUniformBufferCount();
    assert(totalBufferCount != 0);

    std::vector<VkDescriptorSetLayout> layouts = mUniformHandlers.unrollDescriptorLayouts();

    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = mUniformDescriptorPool;
        allocInfo.descriptorSetCount = totalBufferCount;
        allocInfo.pSetLayouts = layouts.data();
    }
    
    std::vector<VkDescriptorSet> uniformDescriptorSetsTemp(totalBufferCount, VK_NULL_HANDLE);
    if(vkAllocateDescriptorSets(mLogicalDevice, &allocInfo, uniformDescriptorSetsTemp.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate uniform descriptor sets");
    }

    std::vector<VkDescriptorBufferInfo> bufferInfos = mUniformHandlers.unrollDescriptorBufferInfo();
    std::vector<UniformHandlerCollection::ExtraInfo> extraInfos = mUniformHandlers.unrollExtraInfo();

    std::vector<VkWriteDescriptorSet> setWriters;
    assert(uniformDescriptorSetsTemp.size() == bufferInfos.size() && bufferInfos.size() == extraInfos.size());
    setWriters.reserve(bufferInfos.size());

    for(size_t i = 0; i < bufferInfos.size(); ++i){
        setWriters.emplace_back(
            VkWriteDescriptorSet{
                /* sType = */ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                /* pNext = */ nullptr, 
                /* dstSet = */ uniformDescriptorSetsTemp[i],
                /* dstBinding = */ extraInfos[i].binding,
                /* dstArrayElement = */ 0,
                /* descriptorCount = */ 1,
                /* descriptorType = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* pImageInfo = */ nullptr,
                /* pBufferInfo = */ &bufferInfos[i],
                /* pTexelBufferView = */ nullptr
            }
        );
    }
    vkUpdateDescriptorSets(mLogicalDevice, setWriters.size(), setWriters.data(), 0, nullptr);

    // Store descriptor sets in a more convinient data structure associating associated by swap chain index
    bundleDescriptorSets(uniformDescriptorSetsTemp);
}

void VulkanGraphicsApp::bundleDescriptorSets(const std::vector<VkDescriptorSet>& aUnrolled) {
}

