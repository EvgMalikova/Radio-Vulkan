
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/structs_vk.hpp>
#endif




#include <vulkanStuct.h>
namespace pv {
  
  struct QueueFamilyIndices {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
  
      bool isComplete() {
          return graphicsFamily.has_value() && presentFamily.has_value();
      }
  };
  
  struct SwapChainSupportDetails {
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
  };
  
  //Helpers
    bool isDeviceSuitable(VkPhysicalDevice device, std::vector<const char*> deviceExtensions,VkSurfaceKHR surface);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface) ;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR surface);
    
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* win) ;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    
    VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device) ;
    std::vector<char> readFile(const std::string& filename);
    
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, 
    			VkCommandBufferLevel level, 
    			uint32_t bufferCount);
    VkCommandPoolCreateInfo commandPoolCreateInfo();
    VkCommandBufferBeginInfo commandBufferBeginInfo();
    void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkImageSubresourceRange subresourceRange);
    
    
    VkResult  createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) ;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkDevice device, VkQueue queueGCT) ;
  /*Graphics helper*/
  class pvContext
  {
  public:
    uint32_t queueFamilyIndex; // Used to check if compute and graphics queue
                                // families differ and require additional barriers
    pvContext(){};
    ~pvContext(){};
    
  
    
    VkInstance         m_instance{VK_NULL_HANDLE};
    VkDevice           m_device{VK_NULL_HANDLE};
    VkPhysicalDevice   m_physicalDevice{VK_NULL_HANDLE};
    
    uint32_t           m_apiMajor = 0;
    uint32_t           m_apiMinor = 0;
    VkQueue m_queueGCT; /* graphics queue*/
    void SetUpQueue();
    
    /* Set up queue and logical device
     * TODO: make as a part of context*/
   
    
    // All the queues (if present) is distinct from each other
    //Queue m_queueGCT;  // for Graphics/Compute/Transfer (must exist)
    //Queue m_queueT;    // for pure async Transfer Queue (can exist, supports at least transfer)
    //Queue m_queueC;    // for async Compute (can exist, supports at least compute)
    
    /*
    std::vector<VkPhysicalDevice> gpuPhysicalDevices;
    std::vector<VkQueue> queues;
    std::vector<VkDevice> devices;*/
    
  };
  uint32_t getMemoryTypeIndex(uint32_t typeBits,
  VkMemoryPropertyFlags properties,VkPhysicalDevice dev);
  uint32_t getMemoryTypeIndex(uint32_t typeBits,
  VkMemoryPropertyFlags properties,pvContext context) ;
  
  VkResult createBuffer(VkBufferUsageFlags usageFlags,
  VkMemoryPropertyFlags memoryPropertyFlags,
  VkBuffer *buffer, VkDeviceMemory *memory,
  VkDeviceSize size, pvContext context, void *data = nullptr);
  
  void submitWork(VkCommandBuffer cmdBuffer, VkQueue queue,  VkDevice device) ;
  
  VkCommandBuffer AllocateAndBeginOneTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool);
  
  void EndSubmitWaitAndFreeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkCommandBuffer& cmdBuffer);
  VkDeviceAddress GetBufferDeviceAddress(VkDevice device, VkBuffer buffer);
  
  
 
}

