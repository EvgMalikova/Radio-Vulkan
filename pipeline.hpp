#ifndef SET_UP
#define SET_UP
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/descriptorsets_vk.hpp>
#include <nvvk/raytraceKHR_vk.hpp> // For nvvk::RaytracingBuilderKHR
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#endif
//Camera parameters

#include "camera.hpp"
#include "context.hpp"
#include "helpers.hpp"
#include "renderer.hpp"
#include <random>
#include <vector>

#include <vulkanStuct.h>

class PipelineBase {
public:
    PipelineBase() { m_interactive = false; };
    ~PipelineBase() {};
  
    void submitBuffers(pv2::Context context, int i);
    void SaveImage (const char* filename, char*pixels, int imageSize, int texWidth, int texHeight);
    
 
    
   
    
    PipelineBase(bool interact)
    {

        m_interactive = interact;
        //m_context.SetInteractive(interact);

        //m_context.Initialize();
    };
    void SetSize(uint32_t w, uint32_t h)
    {
        render_width = w;
        render_height = h;
    };

    void CleanUp(pv2::Context context)
    {
        vkFreeCommandBuffers(context.m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.resize(0);
        vkDestroyPipeline(context.m_device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(context.m_device, m_pipelineLayout, nullptr);
    }

    virtual void CreateDescriptorSetLayout(pv2::Context context) {}; //this will be different for rasterization and ray-tracing
    virtual void CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren) {};
    virtual void CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize) {};
    virtual void CreateDescriptorPool(pv2::Context context, int size) {};
    virtual void CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam) {};

    void CreateCommandPool(pv2::Context context, pv2::RenderBase ren);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, pv2::Context context);

    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkPipeline m_graphicsPipeline;
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorPool m_descriptorPool;
		
protected:
    bool m_interactive;
    // pv2::Context m_context;

    uint32_t render_width;
    uint32_t render_height;
    int world_size;
    int world_rank;
    int m_type;

};

class PipelineRasterize : public PipelineBase {
public:
    PipelineRasterize() {
        //Create acceleration structures
        
    };
    ~PipelineRasterize() {};
    
    

    virtual void CreateDescriptorSetLayout(pv2::Context context); //this will be different for rasterization and ray-tracing
    virtual void CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren);
    virtual void CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize);

    virtual void CreateDescriptorPool(pv2::Context context, int size);
    virtual void CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam);
    
private:
    std::string vertShaderName = "shaders/21_shader_ubo.vert.spv";
    std::string fragShaderName = "shaders/21_shader_ubo.frag.spv";
};

class PipelineRayTrace : public PipelineRasterize {
public:
    

    
    struct RayTracingScratchBuffer
    {
    	uint64_t deviceAddress = 0;
    	VkBuffer handle = VK_NULL_HANDLE;
    	VkDeviceMemory memory = VK_NULL_HANDLE;
    };
    
    // Ray tracing acceleration structure
    struct AccelerationStructure {
    	VkAccelerationStructureKHR handle;
    	uint64_t deviceAddress = 0;
    	VkDeviceMemory memory;
    	VkBuffer buffer;
    };
    class ShaderBindingTable  {
    	public:
    	    VkBuffer buffer = VK_NULL_HANDLE;
    	    VkDeviceMemory memory = VK_NULL_HANDLE;
    	    VkDeviceSize size = 0;
    	    		VkDeviceSize alignment = 0;
    	    		void* mapped = nullptr;
    	    		VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    	    			{
    	    				return vkMapMemory(device, memory, offset, size, 0, &mapped);
    	    			}
    		VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
    	};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    	struct ShaderBindingTables {
    		ShaderBindingTable raygen;
    		ShaderBindingTable miss;
    		ShaderBindingTable hit;
    	} shaderBindingTables;
    
    PipelineRayTrace() {};
    ~PipelineRayTrace() {};
    
    std::vector<VkDescriptorImageInfo> m_storageImageDescriptor;
    std::vector<VkDescriptorBufferInfo> m_vertexBufferDescriptor;
    
    
    void CreateImageDescriptor(VkImageView view)
    {
        VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL };
        m_storageImageDescriptor.push_back(storageImageDescriptor);
        std::cout<<"View is created "<<std::endl;
    }
    void CreateBufferDescriptor(VkBuffer vertex){
        VkDescriptorBufferInfo vertexBufferDescriptor{ vertex, 0, VK_WHOLE_SIZE };
        m_vertexBufferDescriptor.push_back(vertexBufferDescriptor);
        std::cout<<"Buffer is created "<<std::endl;
    }
    
    void CreateShaderBindingTable(pv2::Context context, ShaderBindingTable& shaderBindingTable, uint32_t handleCount);
    
    
    void CreateShaderBindingTables(pv2::Context context);
    
    virtual void CreateDescriptorSetLayout(pv2::Context context);
    virtual void CreateDescriptorSets(pv2::Context context,int size, SimpCamera cam);
    
    virtual void CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren);
    virtual void CreateDescriptorPool(pv2::Context context, int size);
    //virtual void CreateDescriptorSetLayout(pv2::Context context);- not implemented as far

    #ifdef USE_NVVK
    //void buildBlas(const std::vector<nvvk::RaytracingBuilderKHR::BlasInput>&        input, VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
    
    nvvk::RaytracingBuilderKHR::BlasInput sphereToVkGeometryKHR(pv2::Context context, VkBuffer vertexBuffer,int num_vertices);
    #endif
    void CreateBottomLevelAccelerationStructure(pv2::Context context, VkBuffer vertexBuffer,int num_vertices);
    
void CreateTopLevelAccelerationStructure(pv2::Context context);
void CreateAccelerationStructureBuffer(pv2::Context context,AccelerationStructure &accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
RayTracingScratchBuffer CreateScratchBuffer(pv2::Context context, VkDeviceSize size);
	
     AccelerationStructure bottomLevelAS{};
     AccelerationStructure topLevelAS{};
     

     private:
         void CreateAccelerationStructure(pv2::Context context,AccelerationStructure& accelerationStructure, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
         void DeleteAccelerationStructure(pv2::Context context,AccelerationStructure& accelerationStructure);
      
       std::string raygenShaderName = "shaders/raygen.rgen.spv";
       std::string missShaderName = "shaders/miss.rmiss.spv";
       std::string rintShaderName = "shaders/raytrace.rint.spv";
       std::string rchitShaderName = "shaders/closesthit.rchit.spv";
       
       VkStridedDeviceAddressRegionKHR getSbtEntryStridedDeviceAddressRegion(pv2::Context context,VkBuffer buffer, uint32_t handleCount);
       uint32_t alignedSize(uint32_t value, uint32_t alignment);
    
};




#endif //