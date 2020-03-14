#ifndef VULKAN_GRAPHICS_APP_H_
#define VULKAN_GRAPHICS_APP_H_
#include "VulkanSetupBaseApp.h"
#include "vkutils/vkutils.h"
#include "data/VertexGeometry.h"
#include "data/UniformBuffer.h"
#include <map>

class VulkanGraphicsApp : public VulkanSetupBaseApp{
 public:
    
    void init();
    void cleanup();
    
    const VkExtent2D& getFramebufferSize() const;

 protected:

    void render(glm::mat4 m0, glm::mat4 m1, glm::mat4 p);

    void setVertexInput(
       const VkVertexInputBindingDescription& aBindingDescription,
       const std::vector<VkVertexInputAttributeDescription>& aAttributeDescriptions
    );

    void setVertexBuffer(const VkBuffer& aBuffer, size_t aVertexCount);

    void setVertexShader(const std::string& aShaderName, const VkShaderModule& aShaderModule);
    void setFragmentShader(const std::string& aShaderName, const VkShaderModule& aShaderModule);

    /** Add a new uniform to the graphics pipeline via the uniform handler interface class.
     * If a uniform handler already exists for the given binding point, the existing handler is freed and replaced. 
     * 
     * Arguments:
     *   aBindingPoint: The bind point for the uniform to be bound
     *   aUniformData: shared pointer to class implementing UniformDataInterface
     *   aStages: Optional bitmask of shader stages to expose the uniform to. Defaults to vertex and fragment stages.
    */
    void addUniform(uint32_t aBindPoint, UniformDataInterfacePtr aUniformData, VkShaderStageFlags aStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    size_t mFrameNumber = 0;

 private:

    void initRenderPipeline();
    void initFramebuffers();
    void initCommands();
    void initSync();
    
    void resetRenderSetup();
    void cleanupSwapchainDependents();

    void initUniformBuffer();
    void initUniformDescriptorPool();
    void initUniformDescriptorSets();

    void initDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    const static int IN_FLIGHT_FRAME_LIMIT = 2;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishSemaphores;
    std::vector<VkFence> mInFlightFences;

    vkutils::BasicVulkanRenderPipeline mRenderPipeline;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCommandBuffers;

    std::unordered_map<std::string, VkShaderModule> mShaderModules;
    std::string mVertexKey;
    std::string mFragmentKey;

    bool mVertexInputsHaveBeenSet = false;
    VkVertexInputBindingDescription mBindingDescription = {};
    std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;
    VkBuffer mVertexBuffer = VK_NULL_HANDLE;
    size_t mVertexCount = 0U;


    UniformBuffer mUniformBuffer;
    VkDeviceSize mTotalUniformDescriptorSetCount = 0;
    VkDescriptorPool mUniformDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> mUniformDescriptorSetLayouts;
    std::vector<VkDescriptorSet> mUniformDescriptorSets;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};

#endif
