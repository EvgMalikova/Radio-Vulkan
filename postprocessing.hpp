#ifndef POSTPROCESS_UP
#define POSTPROCESS_UP

#include "pipeline.hpp"


class MPICollect: public PipelineBase {
public:
    MPICollect() { m_interactive = false; };
    ~MPICollect() {};
    //For final MPI processing
    
    void CreateMPIDescriptorSetLayout(pv2::Context context);
    void CreateMPIGraphicsPipeline(pv2::Context context, pv2::RenderBase ren);
    void CreateTextureImageView(pv2::Context context) ;
    void CreateTextureSampler(pv2::Context context);
    void CreateTextureImage(pv2::Context context, char* pixels, int texWidth, int texHeight, int texChannels) ;
    
    void CreateVertexBuffer(pv2::Context context);
    void CreateIndexBuffer(pv2::Context context);
    void CreateMPIDescriptorSets(pv2::Context context, int size, SimpCamera cam);
    void CreateMPIDescriptorPool(pv2::Context context, int size);
    void CreateMPICommandBuffers(pv2::Context context, pv2::RenderBase ren);
    void CreateCommandBuffers(pv2::Context context);
    void RecordCommandBuffer(pv2::RenderBase ren);
    
    
    //Helpers
    void endSingleTimeCommands(VkCommandBuffer commandBuffer,pv2::Context context);
    VkCommandBuffer beginSingleTimeCommands(pv2::Context context ) ;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,pv2::Context context);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, pv2::Context context);
    
    VkImageView m_textureImageView;
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkSampler m_textureSampler;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
   
    
   void SetCommandPool(VkCommandPool& p){
       m_commandPool=p;
   }
   
    private:
    std::string vertMPIShaderName = "shaders/26_shader_textures.vert.spv";
    std::string fragMPIShaderName = "shaders/26_shader_textures.frag.spv";
};




#endif //