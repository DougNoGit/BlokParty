#include "utils/common.h"
#include "vkutils/VulkanDevices.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <cstring>

/* TODO: This class uses very naive memory allocation which will be brutally inefficient at scale. */

template<typename VertexType>
class VertexAttributeBuffer
{
 public:
    using vertex_type = VertexType; 

    VertexAttributeBuffer(){}
    explicit VertexAttributeBuffer(const std::vector<VertexType>& aVertices, const VulkanDeviceHandlePair& aDevicePair = {}, bool aSkipDeviceUpload = false) : mCpuVertexData(aVertices) {
        if(!aDevicePair.isNull() && !aSkipDeviceUpload) updateDevice(aDevicePair); 
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

    enum DeviceSyncStateEnum{
        DEVICE_EMPTY,
        DEVICE_OUT_OF_SYNC,
        DEVICE_IN_SYNC,
        CPU_DATA_FLUSHED
    };

    virtual DeviceSyncStateEnum getDeviceSyncState() const {return(mDeviceSyncState);}
    virtual void updateDevice(VulkanDeviceHandlePair aDevicePair = {});

    VulkanDeviceHandlePair getCurrentDevice() const {return(mCurrentDevice);}

    const VkBuffer& getBuffer() const {return(mVertexBuffer);}
    const VkBuffer& handle() const {return(mVertexBuffer);}

    virtual void freeBuffer() {_cleanup();}

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

    virtual void setupDeviceUpload(VulkanDeviceHandlePair aDevicePair);
    virtual void uploadToDevice(VulkanDeviceHandlePair aDevicePair);
    virtual void finalizeDeviceUpload(VulkanDeviceHandlePair aDevicePair);


    std::vector<VertexType> mCpuVertexData;
    DeviceSyncStateEnum mDeviceSyncState = DEVICE_EMPTY;
    

    VkBuffer mVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize mCurrentBufferSize = 0U;
    VulkanDeviceHandlePair mCurrentDevice = {VK_NULL_HANDLE, VK_NULL_HANDLE};

 private:
    void _cleanup();

    VkDeviceSize mCurrentDeviceAllocSize_ = 0U; 
};

template<typename VertexType> void VertexAttributeBuffer<VertexType>::updateDevice(VulkanDeviceHandlePair aDevicePair){
    if(!aDevicePair.isNull() && aDevicePair != mCurrentDevice){
        _cleanup();
        mCurrentDevice = aDevicePair; 
    }

    if(mCurrentDevice.isNull()){
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
        VkBufferCreateInfo createInfo;
        {
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.size = requiredSize;
            createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0U;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        if(vkCreateBuffer(aDevicePair.device, &createInfo, nullptr, &mVertexBuffer) != VK_SUCCESS){
            throw std::runtime_error("Failed to create vertex buffer!"); 
        }
    }

}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::uploadToDevice(VulkanDeviceHandlePair aDevicePair){
    VkDeviceSize requiredSize = sizeof(VertexType) * mCpuVertexData.size();
    
    if(mDeviceSyncState == DEVICE_EMPTY || requiredSize != mCurrentBufferSize){
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(aDevicePair.device, mVertexBuffer, &memRequirements);

        VkPhysicalDeviceMemoryProperties memoryProps;
        vkGetPhysicalDeviceMemoryProperties(aDevicePair.physicalDevice, &memoryProps);

        uint32_t memTypeIndex = VK_MAX_MEMORY_TYPES; 
        for(uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i){
            if(memRequirements.memoryTypeBits & (1 << i) && memoryProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
                memTypeIndex = i;
                break;
            }
        }
        if(memTypeIndex == VK_MAX_MEMORY_TYPES){
            throw std::runtime_error("No compatible memory type could be found for uploading vertex attribute buffer to device!");
        }

        VkMemoryAllocateInfo allocInfo;
        {
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.pNext = nullptr;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = memTypeIndex;
        }

        mCurrentDeviceAllocSize_ = memRequirements.size;

        if(vkAllocateMemory(aDevicePair.device, &allocInfo, nullptr, &mVertexBufferMemory) != VK_SUCCESS){
            throw std::runtime_error("Failed to allocate memory for vertex attribute buffer!");
        }

        vkBindBufferMemory(aDevicePair.device, mVertexBuffer, mVertexBufferMemory, 0);
        mCurrentBufferSize = requiredSize;
    }

    void* mappedPtr = nullptr;
    VkResult mapResult = vkMapMemory(aDevicePair.device, mVertexBufferMemory, 0, mCurrentBufferSize , 0, &mappedPtr);
    if(mapResult != VK_SUCCESS || mappedPtr == nullptr) throw std::runtime_error("Failed to map memory during vertex attribute buffer upload!");
    {
        memcpy(mappedPtr, mCpuVertexData.data(), mCurrentBufferSize);

        VkMappedMemoryRange mappedMemRange;
        {
            mappedMemRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemRange.pNext = nullptr;
            mappedMemRange.memory = mVertexBufferMemory;
            mappedMemRange.offset = 0;
            mappedMemRange.size = mCurrentDeviceAllocSize_;
        }
        if(vkFlushMappedMemoryRanges(aDevicePair.device, 1, &mappedMemRange) != VK_SUCCESS){
            throw std::runtime_error("Failed to flush mapped memory during vertex attribute buffer upload!");
        }
    }vkUnmapMemory(aDevicePair.device, mVertexBufferMemory); mappedPtr = nullptr;

}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::finalizeDeviceUpload(VulkanDeviceHandlePair aDevicePair){
    mDeviceSyncState = DEVICE_IN_SYNC;
}

template<typename VertexType>
void VertexAttributeBuffer<VertexType>::_cleanup(){
    if(mVertexBuffer != VK_NULL_HANDLE){
        vkDestroyBuffer(mCurrentDevice.device, mVertexBuffer, nullptr);
        mVertexBuffer = VK_NULL_HANDLE;
    }
    if(mVertexBufferMemory != VK_NULL_HANDLE){
        vkFreeMemory(mCurrentDevice.device, mVertexBufferMemory, nullptr);
        mVertexBufferMemory = VK_NULL_HANDLE;
    }
    mCurrentBufferSize = 0U;
    mDeviceSyncState = DEVICE_EMPTY;
    
}
