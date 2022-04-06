
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/structs_vk.hpp>
#endif
#include <optional>
#include <vector>
#include <vulkanStuct.h>
#include <cassert>

#define CHECK_VK(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << pv::error(res)<< "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

namespace pv {
std::string error(VkResult errorCode);
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
void createImage3D(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) ;
VkImageView createImageView3D(VkDevice device, VkImage image, VkFormat format);
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format) ;
void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) ;

void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
			VkDescriptorType type,
			VkShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount = 1)
		{
			VkDescriptorSetLayoutBinding setLayoutBinding {};
			setLayoutBinding.descriptorType = type;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorCount = descriptorCount;
			return setLayoutBinding;
		}
		
inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
			const std::vector<VkDescriptorPoolSize>& poolSizes,
			uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolInfo.pPoolSizes = poolSizes.data();
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}
inline VkWriteDescriptorSet WriteDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo *imageInfo,
			uint32_t descriptorCount = 1)
		{
			VkWriteDescriptorSet writeDescriptorSet {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = imageInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		}
		inline VkWriteDescriptorSet WriteDescriptorSet(
					VkDescriptorSet dstSet,
					VkDescriptorType type,
					uint32_t binding,
					VkDescriptorBufferInfo* bufferInfo,
					uint32_t descriptorCount = 1)
				{
					VkWriteDescriptorSet writeDescriptorSet {};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = dstSet;
					writeDescriptorSet.descriptorType = type;
					writeDescriptorSet.dstBinding = binding;
					writeDescriptorSet.pBufferInfo = bufferInfo;
					writeDescriptorSet.descriptorCount = descriptorCount;
					return writeDescriptorSet;
				}
				
				inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
							VkDescriptorPool descriptorPool,
							const VkDescriptorSetLayout* pSetLayouts,
							uint32_t descriptorSetCount)
						{
							VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
							descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
							descriptorSetAllocateInfo.descriptorPool = descriptorPool;
							descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
							descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
							return descriptorSetAllocateInfo;
						}
/*inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
			VkCommandPool commandPool, 
			VkCommandBufferLevel level, 
			uint32_t bufferCount)
		{
			VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = commandPool;
			commandBufferAllocateInfo.level = level;
			commandBufferAllocateInfo.commandBufferCount = bufferCount;
			return commandBufferAllocateInfo;
		}*/
void InsertImageMemoryBarrier(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange);
struct QueueGraphicFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//Helpers

void FlushCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free);
uint32_t getMemoryTypeIndex(uint32_t typeBits,
VkMemoryPropertyFlags properties, VkPhysicalDevice dev);

VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice,VkFormat *depthFormat);
bool isDeviceSuitable(VkPhysicalDevice device, std::vector<const char*> deviceExtensions, VkSurfaceKHR surface);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
QueueGraphicFamilyIndices findGraphicsQueueFamilies(VkPhysicalDevice device);
bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
#ifdef USE_GLFW
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* win);
#endif
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device);
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

VkResult createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkDevice device, VkQueue queueGCT);
/*Graphics helper*/
class pvContext {
public:
    uint32_t queueFamilyIndex; // Used to check if compute and graphics queue
        // families differ and require additional barriers
    pvContext() {};
    ~pvContext() {};

    VkInstance m_instance { VK_NULL_HANDLE };
    VkDevice m_device { VK_NULL_HANDLE };
    VkPhysicalDevice m_physicalDevice { VK_NULL_HANDLE };

    uint32_t m_apiMajor = 0;
    uint32_t m_apiMinor = 0;
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
    VkMemoryPropertyFlags properties, pvContext context);

VkResult createBuffer(VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkBuffer* buffer, VkDeviceMemory* memory,
    VkDeviceSize size, pvContext context, void* data = nullptr);

void submitWork(VkCommandBuffer cmdBuffer, VkQueue queue, VkDevice device);

//VkCommandBuffer AllocateAndBeginOneTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool);

void EndSubmitWaitAndFreeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkCommandBuffer& cmdBuffer);
VkDeviceAddress GetBufferDeviceAddress(VkDevice device, VkBuffer buffer);



        
       
}


