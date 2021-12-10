#ifndef CONTEXT
#define CONTEXT
#include <vector>
#include <cstring>

#include <iostream>
#include <fstream>
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/structs_vk.hpp>
#endif




namespace pv2 {
  /*Graphics helper*/
  class Context
  {
  public:
    uint32_t queueFamilyIndex; // Used to check if compute and graphics queue
                                // families differ and require additional barriers
   Context(){
     #ifdef LOG_All
     m_enableValidationLayers=true;
     #else
     m_enableValidationLayers=false;
     #endif
     m_interactive=true;
     
   };
    ~Context(){};
    
    void Initialize()
    {CreateInstance();
    }
    
  
    
    VkInstance         m_instance{VK_NULL_HANDLE};
    VkDevice           m_device{VK_NULL_HANDLE};
    VkPhysicalDevice   m_physicalDevice{VK_NULL_HANDLE};
    
    uint32_t           m_apiMajor = 0;
    uint32_t           m_apiMinor = 0;
    /*
    * `Queue m_queueGCT` : Graphics/Compute/Transfer Queue + family index
    * `Queue m_queueT` : async Transfer Queue + family index
    * `Queue m_queueC` : Compute Queue + family index
    */
    VkQueue m_queueGCT; 
    VkQueue m_queueT;
    VkQueue m_queueP; //present queue
    
    
    void SetUpQueue();
    
    VkSurfaceKHR m_surface;
    
    VkDebugUtilsMessengerEXT m_debugMessenger;
    
 

  
  
 
  void AddExtension(const char* name);
  void SetInteractive(bool v){
    m_interactive=v;
    AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    };
  
  
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  
        return VK_FALSE;
    }
    
    void CleanUp();
    
    void PickPhysicalDevice(VkSurfaceKHR surface);
    void CreateLogicalDevice(VkSurfaceKHR surface);
    
 
 private:
  void CreateInstance();
  bool CheckValidationLayerSupport();
   

   
   void SetupDebugMessenger();
   std::vector<const char*> GetRequiredExtensions();
   std::vector<const char*> GetGLFWExtensions();
   void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
   bool m_enableValidationLayers;
   const std::vector<const char*> m_validationLayers = {
       "VK_LAYER_KHRONOS_validation"
   };
   //TODO add switch also to "VK_LAYER_LUNARG_standard_validation"
   std::vector<const char*> m_deviceExtensions;// = {
     //  VK_KHR_SWAPCHAIN_EXTENSION_NAME
   //};
   
   
   bool m_interactive;
};
}
#endif // 