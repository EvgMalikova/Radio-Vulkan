#ifndef SET_UP
#define SET_UP



#include "nvvk/context_vk.hpp"
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#include <nvvk/descriptorsets_vk.hpp>
#include <nvvk/raytraceKHR_vk.hpp>        // For nvvk::RaytracingBuilderKHR
//Camera parameters

#include <random>
#include <vector>
#include "camera.hpp"
#include "helpers.hpp"
#include "context.hpp"
#include "renderer.hpp"

#include <vulkanStuct.h>

class PipelineBase
{
public:
  
   PipelineBase(){m_interactive=false;};
   ~PipelineBase(){};
   PipelineBase(bool interact)
    {
    
     m_interactive=interact;
     //m_context.SetInteractive(interact);
     
     //m_context.Initialize();
     
    
     
     
     
     
    };
      void SetSize(uint32_t w, uint32_t h)
      {
        render_width=w;
        render_height=h;
      };
      
      void CleanUp(pv2::Context context)
      {
        vkFreeCommandBuffers(context.m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()),m_commandBuffers.data());
        
        vkDestroyPipeline(context.m_device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(context.m_device, m_pipelineLayout, nullptr);
      }
      
      virtual void CreateDescriptorSetLayout(pv2::Context context){}; //this will be different for rasterization and ray-tracing
      virtual void CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren) {};
      virtual void CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize){};
     virtual void CreateDescriptorPool(pv2::Context context, int size){};
      virtual void CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam){};
      
      void CreateCommandPool(pv2::Context context, pv2::RenderBase ren) ;
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
     
      
      uint32_t  render_width;
      uint32_t render_height;
      int world_size;
      int world_rank;
      int m_type;
};

class PipelineRasterize: public PipelineBase
{
public:
  
   PipelineRasterize(){};
   ~PipelineRasterize(){};
  
    
      virtual void CreateDescriptorSetLayout(pv2::Context context); //this will be different for rasterization and ray-tracing
      virtual void CreateGraphicsPipeline(pv2::Context context,pv2::RenderBase ren);
      virtual void CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize);
      
      
      virtual void CreateDescriptorPool(pv2::Context context, int size);
      virtual void CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam);
      
    private:
      std::string vertShaderName="shaders/21_shader_ubo.vert.spv";
      std::string fragShaderName="shaders/21_shader_ubo.frag.spv";
};

//Program set UP
class PipelineSetUp: public PipelineBase
{
public:
  
  //TODO: remove
  
  nvvk::Image image ;
  nvvk::Image imageLinear;
  
  VkPipeline   rtPipeline; /*pipeline itself*/
  
  
  //TDO; move to protected
  #ifdef USE_NVVK
  nvvk::Context context;
  #else
  pv::pvContext context; //altenative context
  #endif;
  
 
  
  //As far separate them
  //nvvk::Context context;
  //pv::pvContext pvcontext; 
  
  VkCommandPool cmdPool;
  
  nvvk::ResourceAllocatorDedicated allocator;
  nvvk::DescriptorSetContainer descriptorSetContainer;
  
  PipelineSetUp();
   
   PipelineSetUp(bool interact)
   {
   
    PipelineSetUp();
    
    
   };
   
   ~PipelineSetUp()
   {
     allocator.unmap(imageLinear);
        
      
        vkDestroyPipeline(context, rtPipeline, nullptr);
        
        descriptorSetContainer.deinit();
      
        
        allocator.destroy(imageLinear);
        vkDestroyImageView(context, imageView, nullptr);
        allocator.destroy(image);
        allocator.deinit();
       
        context.deinit();  // Don't forget to clean up at the end of the program!
   }
   
   virtual void Initialize();
   virtual void SetPipelineProperties()/*Second stage: get properties necessary for furter construction*/
    {
    };
    
    
    virtual void CreateShaders(){};
    virtual void CreateDescriptorSet(nvvk::Buffer vertexBuffer){};//, nvvk::Buffer indexBuffer){};
    
    
    void CreateImageBarriers(VkCommandBuffer uploadCmdBuffer);
    void CreateImage(); /* A set of operations for image to write ray-tracing output to*/
    
    
    void CreatePool();
    void SaveImage();/*Writes image to file*/
    void CopyImage(VkCommandBuffer cmdBuffer);/*Writes image to host*/
    
    
   protected:
    
    /*TODO further move to other place*/
    
    SimpCamera camera;
    
    /*For output image*/
    VkImageView imageView;
  
   
};
  



class RayTrace: public PipelineSetUp
{
public:

  
  
  /*Set of blases*/
  std::vector<nvvk::RaytracingBuilderKHR::BlasInput> blases;
  
  RayTrace()
  {
    m_Dlayout=Basic;
    m_numBind=3;
    m_maxDepth=1;
    
    
    
  };
  ~RayTrace(){
    
   // allocator.unmap(imageLinear);
    
    allocator.destroy(rtSBTBuffer);
  //  vkDestroyPipeline(context, rtPipeline, nullptr);
    for(VkShaderModule& shaderModule : modules)
    {
      vkDestroyShaderModule(context, shaderModule, nullptr);
    }
 //   descriptorSetContainer.deinit();
    raytracingBuilder.destroy();
    
 //   allocator.destroy(imageLinear);
 //   vkDestroyImageView(context, imageView, nullptr);
 //   allocator.destroy(image);
 //   allocator.deinit();
   
 //   context.deinit();  // Don't forget to clean up at the end of the program!
   
  };
  
  static const size_t  NUM_C_HIT_SHADERS = 1;
   enum DataLayout{
     Basic, /**/
     Advanced
   };
   
   enum Bindings{
     BIND_IMAGEDATA, //0
     BIND_TLAS, //1
     BIND_VERTICES, //2
     BIND_INDICES //3 //TODO remove
   };
  
 virtual void Initialize();
 
  virtual void SetPipelineProperties();
  virtual void CreateShaders();
  virtual void CreateDescriptorSet(nvvk::Buffer vertexBuffer);//, nvvk::Buffer indexBuffer);
  
  void STBCreate();
  

  void CreateBlas()
  {
    raytracingBuilder.setup(context, &allocator, context.m_queueGCT);
    raytracingBuilder.buildBlas(blases,VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
  }
  void CreateTlas()
  {
    //VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR
    //                                        | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    
    // Create 441 instances with random rotations pointing to BLAS 0, and build these instances into a TLAS:
    std::vector<VkAccelerationStructureInstanceKHR> instances;
    std::default_random_engine                      randomEngine;  // The random number generator
    std::uniform_real_distribution<float>           uniformDist(-0.5f, 0.5f);
    std::uniform_int_distribution<int>              uniformIntDist(0, 8);
    
        nvmath::mat4f transform(1);
        //transform.translate(nvmath::vec3f(float(x), float(y), 0.0f));
        transform.scale(3.0f );
       // transform.rotate(uniformDist(randomEngine), nvmath::vec3f(0.0f, 1.0f, 0.0f));
       // transform.rotate(uniformDist(randomEngine), nvmath::vec3f(1.0f, 0.0f, 0.0f));
       // transform.translate(nvmath::vec3f(0.0f, -1.0f, 0.0f));
    
        VkAccelerationStructureInstanceKHR instance{};
        instance.transform           = nvvk::toTransformMatrixKHR(transform);
        instance.instanceCustomIndex = 0;  // 24 bits accessible to ray shaders via rayQueryGetIntersectionInstanceCustomIndexEXT
        // The address of the BLAS in `blases` that this instance points to
        instance.accelerationStructureReference = raytracingBuilder.getBlasDeviceAddress(0);
        // Used for a shader offset index, accessible via rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;  // How to trace this instance
        instance.mask  = 0xFF;
        instances.push_back(instance);
     
    raytracingBuilder.buildTlas(instances, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
  }
  
  void RayTracing(VkCommandBuffer& cmdBuffer)
  {
    vkCmdTraceRaysKHR(cmdBuffer,           // Command buffer
    &sbtRayGenRegion,    // Region of memory with ray generation groups
    &sbtMissRegion,      // Region of memory with miss groups
    &sbtHitRegion,       // Region of memory with hit groups
    &sbtCallableRegion,  // Region of memory with callable groups
    render_width,        // Width of dispatch
    render_height,       // Height of dispatch
    1);                  // Depth of dispatch
    
  }
  
private:
  DataLayout m_Dlayout;
  int m_numBind;
  int m_maxDepth;

  
  std::array<VkShaderModule, 2 + NUM_C_HIT_SHADERS+1> modules;
  std::array<VkPipelineShaderStageCreateInfo, 2 + NUM_C_HIT_SHADERS+1> stages;  // Pointers to shaders
  std::array<VkRayTracingShaderGroupCreateInfoKHR, 2 + NUM_C_HIT_SHADERS> groups;
  
  VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtPipelineProperties; /*pipeline properties*/
  
  /*Builder*/
  nvvk::RaytracingBuilderKHR raytracingBuilder;
  
  

  
  
  nvvk::Buffer rtSBTBuffer; /*buffer*/
  
  VkStridedDeviceAddressRegionKHR sbtRayGenRegion, sbtMissRegion, sbtHitRegion, sbtCallableRegion;
  

  
  
  
  
  
};

class Rasterisation: public PipelineSetUp
{
  public:
  enum DataLayout{
    Basic, /**/
    Advanced
  };
  
  enum Bindings{
      BIND_IMAGEDATA, //0
      BIND_VERTICES //1
    };
    
    Rasterisation()
    {
      m_Dlayout=Basic;
      m_numBind=1;
      
    }
    ~Rasterisation()
    {
      
    }
  private:
    DataLayout m_Dlayout;
    int m_numBind;
};

#endif // 