#include "context.hpp"

#define GLFW_INCLUDE_VULKAN
#include "helpers.hpp"
#include <GLFW/glfw3.h>
#include <optional>
#include <set>

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

void Context::CleanUp()
{
    if (m_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
}

void Context::AddExtension(const char* name)
{
    m_deviceExtensions.push_back(name);
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

std::vector<const char*> Context::GetGLFWExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extension(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extension;
}

std::vector<const char*> Context::GetRequiredExtensions()
{
    std::vector<const char*> extensions;
    if (m_interactive) {
        extensions = GetGLFWExtensions();
    }

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
        }

        if (deviceProperties.deviceType == m_selectedDeviceType)
            m_physicalDevice = device;
    }

    /*
          * There are 3 devices found
          GPUs: Intel(R) UHD Graphics 630 (CFL GT2)
          GPUs: llvmpipe (LLVM 12.0.0, 256 bits)
          GPUs: GeForce RTX 2080
      
          */

    // Run task per device
    //deviceCount = gpuPhysicalDevices.size();
    DEBUG_LOG << "There are " << devGPU << " GPU devices found" << std::endl;
    /*for (int i = 0; i < deviceCount; i++) {
           // initiate procedures for each device
           SetUpQueue(i);
           //contains create command pool
           std::cout << "graphics queue is set" << std::endl;
           GPULoadVertexBuffer(i);
           std::cout << "Work is submitted per queue" << std::endl;
         }*/

    //m_physicalDevice = devices[2];
    if (m_interactive) {
        for (const auto& device : devices) {
            if (pv::isDeviceSuitable(device, m_deviceExtensions, surface)) {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    else { //any device will do for headless rasterization, so we choose a selected one
    }

    DEBUG_LOG << "Proceed with " << m_interactive << " set up" << std::endl;
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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    if (m_interactive) {
        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_queueGCT);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_queueP);
    } else {
        vkGetDeviceQueue(m_device, indicesGr.graphicsFamily.value(), 0, &m_queueGCT);
    }

    DEBUG_LOG << "Created Logical device for type " << m_interactive << std::endl;
}

}