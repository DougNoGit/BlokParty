#ifndef VERTEX_GEOMETRY_H_
#define VERTEX_GEOMETRY_H_

#include "utils/common.h"
#include "DeviceSyncedBuffer.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "vk_mem_alloc.h"

/* TODO need to set up staging and index buffers */

template<typename VertexType>
class VertexAttributeBuffer : public DeviceSyncedBuffer
{
 public:
    using vertex_type = VertexType; 

    VertexAttributeBuffer(){}
    explicit VertexAttributeBuffer(const std::vector<VertexType>& aVertices, VmaAllocator allocator, const VulkanDeviceBundle& aDeviceBundle = {}, bool aSkipDeviceUpload = false) : mCpuVertexData(aVertices), allocator(allocator) {
        if(aDeviceBundle.isValid() && !aSkipDeviceUpload) updateDevice(aDeviceBundle); 
    }

    // Disallow copy to avoid creating invalid instances. VertexAttributeBuffer(s) should be combined with shared_ptrs or made intrusive. 
    VertexAttributeBuffer(const VertexAttributeBuffer& aOther) = delete; 

    virtual ~VertexAttributeBuffer(){
        // Warning if cleanup wasn't explicit to teach responsibility
        if(mVertexBuffer != VK_NULL_HANDLE || mVertexBufferMemory != VK_NULL_HANDLE){
            std::cerr << "Warning! VertexAttributeBuffer object destroyed before buffer was freed" << std::endl;
            _cleanup(); 
        }
    }

    virtual DeviceSyncStateEnum getDeviceSyncState() const override {return(mDeviceSyncState);}
    virtual void updateDevice(const VulkanDeviceBundle& aDevicePair = {}) override;
    virtual VulkanDeviceHandlePair getCurrentDevice() const override {return(mCurrentDevice);}

    virtual const VkBuffer& getBuffer() const override {return(mVertexBuffer);}

    virtual void freeBuffer() override {_cleanup();}

    /** Clears all vertex data held as member data on the class instance, but
     * leaves buffer data on device untouched. After flush, device is not
     * considered 'DEVICE_OUT_OF_SYNC', but instead marked 'CPU_DATA_FLUSHED'
     */
    virtual void flushCpuData() {
        mCpuVertexData.clear();
        mDeviceSyncState = mDeviceSyncState == DEVICE_IN_SYNC ? CPU_DATA_FLUSHED : mDeviceSyncState;
    }

    virtual size_t vertexCount() const {return(mCpuVertexData.size());}
    virtual std::vector<VertexType>& getVertices() {return(mCpuVertexData); mDeviceSyncState = DEVICE_OUT_OF_SYNC;}
    virtual const std::vector<VertexType>& getVertices() const {return(getVerticesConst());}
    virtual const std::vector<VertexType>& getVerticesConst() const {return(mCpuVertexData);}

    virtual void setVertices(const std::vector<VertexType>& aVertices) {mCpuVertexData = aVertices; mDeviceSyncState = DEVICE_OUT_OF_SYNC;}

 protected:

    virtual void setupDeviceUpload(VulkanDeviceHandlePair aDevicePair) override;
    virtual void uploadToDevice(VulkanDeviceHandlePair aDevicePair) override;
    virtual void finalizeDeviceUpload(VulkanDeviceHandlePair aDevicePair) override;


    std::vector<VertexType> mCpuVertexData;
    VmaAllocator allocator;
    VmaAllocation allocation;
    DeviceSyncStateEnum mDeviceSyncState = DEVICE_EMPTY;
    

    VkBuffer mVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize mCurrentBufferSize = 0U;
    VulkanDeviceHandlePair mCurrentDevice = {VK_NULL_HANDLE, VK_NULL_HANDLE};

 private:
    void _cleanup();

    VkDeviceSize _mCurrentDeviceAllocSize = 0U; 
};

template<typename VertexType> 
void VertexAttributeBuffer<VertexType>::updateDevice(const VulkanDeviceBundle& aDeviceBundle){
    if(aDeviceBundle.isValid() && aDeviceBundle != mCurrentDevice){
        _cleanup();
        mCurrentDevice = VulkanDeviceHandlePair(aDeviceBundle); 
    }

    if(!mCurrentDevice.isValid()){
        throw std::runtime_error("Attempting to updateDevice() from vertex attribute buffer with no associated device!");
    }

    setupDeviceUpload(mCurrentDevice);
    uploadToDevice(mCurrentDevice);
    finalizeDeviceUpload(mCurrentDevice);
}


template<typename VertexType>
void VertexAttributeBuffer<VertexType>::setupDeviceUpload(VulkanDeviceHandlePair aDevicePair){
    VkDeviceSize requiredSize = sizeof(VertexType) * mCpuVertexData.size();

    if(mDeviceSyncState == DEVICE_EMPTY || requiredSize != mCurrentBufferSize){
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = requiredSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &mVertexBuffer, &allocation, nullptr);

    }
}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::uploadToDevice(VulkanDeviceHandlePair aDevicePair){
    VkDeviceSize requiredSize = sizeof(VertexType) * mCpuVertexData.size();
    
    mCurrentBufferSize = requiredSize;
    void* mappedPtr = nullptr;
    VkResult mapResult = vmaMapMemory(allocator, allocation, &mappedPtr);
    if(mapResult != VK_SUCCESS || mappedPtr == nullptr) throw std::runtime_error("Failed to map memory during vertex attribute buffer upload!");
    {
        memcpy(mappedPtr, mCpuVertexData.data(), mCurrentBufferSize);

    }vmaUnmapMemory(allocator, allocation); mappedPtr = nullptr;

}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::finalizeDeviceUpload(VulkanDeviceHandlePair aDevicePair){
    mDeviceSyncState = DEVICE_IN_SYNC;
}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::_cleanup(){
    if(mVertexBuffer != VK_NULL_HANDLE){
        vmaDestroyBuffer(allocator, mVertexBuffer, allocation);
        vmaDestroyAllocator(allocator);
        mVertexBuffer = VK_NULL_HANDLE;
    }
    if(mVertexBufferMemory != VK_NULL_HANDLE){
        vkFreeMemory(mCurrentDevice.device, mVertexBufferMemory, nullptr);
        mVertexBufferMemory = VK_NULL_HANDLE;
    }
    mCurrentBufferSize = 0U;
    mDeviceSyncState = DEVICE_EMPTY;
    
}


#endif