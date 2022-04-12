#include "context.hpp"

#define GLFW_INCLUDE_VULKAN
#include "helpers.hpp"

#ifdef USE_MPIRV
#include <mpi.h>
#endif

#ifdef USE_GLFW
#include <GLFW/glfw3.h>
#endif

#include <optional>
#include <set>
#define DEFAULT_FENCE_TIMEOUT 100000000000
#include <vulkanStuct.h>
namespace pv2 {
    
    

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

	VkCommandBuffer Context::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo= pv::commandBufferAllocateInfo(pool, level, 1);
		VkCommandBuffer cmdBuffer;
		CHECK_VK(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &cmdBuffer));
		// If requested, also start recording for the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufInfo = pv::commandBufferBeginInfo();
			CHECK_VK(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
		}
		return cmdBuffer;
	}
			
	

	/**
	* Finish command buffer recording and submit it to a queue
		*/
	void Context::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		CHECK_VK(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;//VK_FLAGS_NONE;
		VkFence fence;
		CHECK_VK(vkCreateFence(m_device, &fenceInfo, nullptr, &fence));
		// Submit to the queue
		CHECK_VK(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		CHECK_VK(vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(m_device, fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(m_device, pool, 1, &commandBuffer);
		}
	}

	

void Context::CleanUp()
{
    if (m_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
}

void Context::AddExtension(const char* name)
{
    m_deviceExtensions.push_back(name);
    DEBUG_LOG<<"Extension added "<<name<<std::endl;
    
}

bool Context::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}
#ifdef USE_GLFW
std::vector<const char*> Context::GetGLFWExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extension(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extension;
}
#endif
std::vector<const char*> Context::GetRequiredExtensions()
{
    std::vector<const char*> extensions;
    #ifdef USE_GLFW
    if (m_interactive) {
        extensions = GetGLFWExtensions();
    }
    #endif
    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


void Context::CreateInstance()
{
    if (m_enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "HVulkanExample";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    } else {
        //#ifdef LOG_All
        SetupDebugMessenger();
        //#endif
    }
}

void Context::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void Context::SetupDebugMessenger()
{
    if (!m_enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Context::PickPhysicalDevice(VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    /* */

    std::cout << "There are " << deviceCount << " devices found" << std::endl;
    int devGPU = 0;
    
    
        std::vector<uint32_t> deviceIDs;
  
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        std::cout << "GPUs: " << deviceProperties.deviceName << " type " << deviceProperties.deviceType << std::endl;

        /*
            * Provided by VK_VERSION_1_0>>
            typedef enum VkPhysicalDeviceType {
                VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
                VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
                VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
                VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
                VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
            } VkPhysicalDeviceType;
            * */
        if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)) {
            devGPU++;
        DEBUG_LOG<<deviceProperties.deviceID<<" Device id "<< world_rank<<std::endl;
deviceIDs.push_back(deviceProperties.deviceID);         
}
        else{ //remove device from list
            devices.erase (devices.begin()+devGPU,devices.begin()+devGPU+1);
        }

        if (deviceProperties.deviceType == m_selectedDeviceType){
            m_physicalDevice = device;
        DEBUG_LOG << "The GPU selected " << deviceProperties.deviceType << std::endl;
    }
    }
   // deviceIDs.resize(devGPU);
   DEBUG_LOG << "There are " << devGPU << " GPU devices found" << std::endl;
/*   #ifdef USE_MPIRV 
   if(devGPU>1){
 if (world_rank ==0)
{
for (int i=1;i<world_size;i++)
{
MPI_Send(deviceIDs.data(), devGPU, MPI_INT, i, world_rank, MPI_COMM_WORLD);
}
}
else{
uint32_t* ids= (uint32_t *) malloc(devGPU);
 MPI_Recv(ids,devGPU, MPI_INT, 0, 0, MPI_COMM_WORLD,     MPI_STATUS_IGNORE);
DEBUG_LOG<<"Process "<<world_rank<<" has received id "<<ids[world_rank]<<std::endl;
}
}
#endif
 
  */      
   

   
    int ndev=world_rank;
  m_physicalDevice=devices[ndev];
 DEBUG_LOG<<"Assign "<<ndev<<" device"<<std::endl;
    if (m_interactive) {
        for (const auto& device : devices) {
            //Check with extension support
            if (pv::isDeviceSuitable(device, m_deviceExtensions, surface)) {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        
        if (m_pipe==pv2::PipelineTYPE::RayTracing){ //Ray-tracing
        
            
                 
                 // Get ray tracing pipeline properties, which will be used later on in the sample
                		rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                 		VkPhysicalDeviceProperties2 deviceProperties2{};
                 		deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                 		deviceProperties2.pNext = &rayTracingPipelineProperties;
                 		vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties2);
                 
                 		// Get acceleration structure properties, which will be used later on in the sample
                 		accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                 		VkPhysicalDeviceFeatures2 deviceFeatures2{};
                 		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                 		deviceFeatures2.pNext = &accelerationStructureFeatures;
                 		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures2);
                 		
                 
                 std::cout << "GPUs for ray-tracing: " << deviceProperties2.properties.deviceName << " type " << deviceProperties2.properties.deviceType << std::endl;
        
             }
    }

   

    DEBUG_LOG << "Proceed with interactiv " << m_interactive << " set up" << std::endl;
}

// Set up queue and logical device



void Context::CreateLogicalDevice(VkSurfaceKHR surface)
{

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    pv::QueueGraphicFamilyIndices indicesGr;
    pv::QueueFamilyIndices indices;

    if (m_interactive) {

        indices = pv::findQueueFamilies(m_physicalDevice, surface);

        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
    } else {
        indicesGr = pv::findGraphicsQueueFamilies(m_physicalDevice);

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indicesGr.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    
   
    

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    
    if (m_pipe==pv2::RayTracing) {
        
    //enabling features for rqay-tracing
   					// Enable features required for ray tracing using feature chaining via pNext		
   					enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
   					enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;
   			
   					enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
   					enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
   					enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;
   			
   					enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
   					enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
   					enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;
   					enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE,
   					enabledAccelerationStructureFeatures.accelerationStructureCaptureReplay = VK_TRUE,
   					enabledAccelerationStructureFeatures.accelerationStructureIndirectBuild = VK_FALSE,
   					enabledAccelerationStructureFeatures.accelerationStructureHostCommands = VK_FALSE,
   					enabledAccelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;
   					
   			
   			createInfo.flags = 0;
   			createInfo.pEnabledFeatures = nullptr;
   			createInfo.pNext = &enabledAccelerationStructureFeatures;
       		} 
    
    DEBUG_LOG << "There are  " <<m_deviceExtensions.size() << " extensions enabled " << std::endl;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    //PFN_vkCreateDevice pvkCreateDevice=(PFN_vkCreateDevice)fpGetInstanceProcAddr(m_device, "vkCreateDevice");
    
    CHECK_VK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) ) ;
    DEBUG_LOG<<"Created logical device"<<std::endl;
    if (m_interactive) {
        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_queueGCT);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_queueP);
    } else {
        vkGetDeviceQueue(m_device, indicesGr.graphicsFamily.value(), 0, &m_queueGCT);
    }

    DEBUG_LOG << "Created Logical device for type " << m_interactive << std::endl;
}

}
