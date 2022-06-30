#ifndef CONTEXT
#define CONTEXT
#include <cstring>
#include <vector>

#include <fstream>
#include <iostream>
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/structs_vk.hpp>
#endif
#include <vulkan/vulkan.h>

namespace pv2 {
    enum PipelineTYPE
    {
        Rasterisation,
        RayTracing
    };

enum InteractionMode
{
    Headless,
    Interactive,
    Streaming
};
/*Graphics helper*/
class Context {
public:
    uint32_t queueFamilyIndex; // Used to check if compute and graphics queue
        // families differ and require additional barriers
    Context()
    {
#ifdef LOG_All
        m_enableValidationLayers = true;
#else
        m_enableValidationLayers = false;
#endif
        m_interactive = true;
    };
    ~Context() {};

    void Initialize()
    {
        CreateInstance();
    }
    bool GetInteractive() { return m_interactive; };

    VkInstance m_instance { VK_NULL_HANDLE };
    VkDevice m_device { VK_NULL_HANDLE };
    VkPhysicalDevice m_physicalDevice { VK_NULL_HANDLE };

    uint32_t m_apiMajor = 0;
    uint32_t m_apiMinor = 0;
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
    void SetInteractive(bool v)
    {
        m_interactive = v;
        if(m_interactive)
        AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    };
    void SetPipelineType(pv2::PipelineTYPE type)
    {
        m_pipe = type;
        
        if(type==pv2::PipelineTYPE::RayTracing)
        {
             AddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                    AddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                    
            	    // VK_KHR_acceleration_structure
            		AddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            		AddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            		AddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            
            		// VK_KHR_ray_tracing_pipeline
            		AddExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
            
            		// VK_KHR_spirv_1_4
            		AddExtension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
            
        }
       
       
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    
    
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free);
    VkCommandBuffer  CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
    
    void CleanUp();

    void PickPhysicalDevice(VkSurfaceKHR surface);
    void CreateLogicalDevice(VkSurfaceKHR surface);
    
    
    //Ray tracing staff
       VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
       VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

private:
    void CreateInstance();
    bool CheckValidationLayerSupport();

    void SetupDebugMessenger();
   
    std::vector<const char*> GetRequiredExtensions();
    #ifdef USE_GLFW
    std::vector<const char*> GetGLFWExtensions();
    #endif
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool m_enableValidationLayers;
    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    //TODO add switch also to "VK_LAYER_LUNARG_standard_validation"
    std::vector<const char*> m_deviceExtensions; // = {
        //  VK_KHR_SWAPCHAIN_EXTENSION_NAME
    //};

    bool m_interactive;
    pv2::PipelineTYPE m_pipe;
    VkPhysicalDeviceType m_selectedDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    
    
   
    
    //void EnableRayTraceFeatures();
    	VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
    				       	VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
    				       	VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
    		
    
};
}
#endif //