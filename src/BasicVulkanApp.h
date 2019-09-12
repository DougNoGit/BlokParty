#ifndef BASIC_VULKAN_APP_H_
#define BASIC_VULKAN_APP_H_

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "vkutils.h"

class BasicVulkanApp{
 public:

    void init();
    void cleanup();

 protected:

    void initGlfw();
    void initVulkan();

    void createVkInstance();
    void initPresentationSurface();
    void initVkDevices();
    void initSwapchain();
    void initSwapchainViews();

    std::vector<std::string> gatherExtensionInfo();
    std::vector<std::string> gatherValidationLayers();

    // Virtual functions for retreving information about the setup of the applications vulkan 
    // instance and devices. Overriding these functions in derived classes allow modifying the 
    // Vulkan setup with minimal amounts of new or re-written logic. 
    virtual const VkApplicationInfo& getAppInfo() const;
    virtual const std::vector<std::string>& getRequiredValidationLayers() const;
    virtual const std::vector<std::string>& getRequestedValidationLayers() const;
    virtual const std::vector<std::string>& getRequiredInstanceExtensions() const;
    virtual const std::vector<std::string>& getRequestedInstanceExtensions() const;
    virtual const std::vector<std::string>& getRequiredDeviceExtensions() const;
    virtual const std::vector<std::string>& getRequestedDeviceExtensions() const;
    // Functions used to determine swap chain configuration
    virtual const VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aFormats) const;
    virtual const VkPresentModeKHR selectPresentationMode(const std::vector<VkPresentModeKHR>& aModes) const;
    virtual const VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR& aCapabilities) const;

    const std::unordered_map<std::string, bool>& getValidationLayersState() const;
    const std::unordered_map<std::string, bool>& getExtensionState() const; 

    VkExtent2D mViewportExtent = {854, 480};

    VkInstance mVkInstance;
    VulkanPhysicalDevice mPhysDevice;
    VulkanDevice mLogicalDevice;
    VkSurfaceKHR mVkSurface;
    VkQueue mGraphicsQueue;
    VkQueue mPresentationQueue;
    VkSwapchainKHR mVkSwapchain; 

    VkSurfaceFormatKHR mSwapchainFormat;
    VkPresentModeKHR mPresentationMode;
    VkExtent2D mSwapchainExtent = {0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t mRequestedImageCount = 0;
    uint32_t mImageCount = 0;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainViews;
    
    

    GLFWwindow* mWindow = nullptr;

 private:

    std::unordered_map<std::string, bool> _mValidationLayers;
    std::unordered_map<std::string, bool> _mExtensions; 
    

};

#endif