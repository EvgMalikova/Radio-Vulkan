#ifndef POSTPROCESS_UP
#define POSTPROCESS_UP

#include "pipeline.hpp"
#include <array>
/*
 * For image processing as a result of MPI
 */
struct TexVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(TexVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(TexVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(TexVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(TexVertex, texCoord);

        return attributeDescriptions;
    }
};



class MPICollect: public PipelineBase {
public:
    MPICollect() { m_interactive = false; m_synthesize=true;};
    ~MPICollect() {};
    
    struct Texture
    	{
    		VkSampler             sampler = VK_NULL_HANDLE;
    		VkImage               image   = VK_NULL_HANDLE;
    		VkImageLayout         imageLayout;
    		VkDeviceMemory        deviceMemory = VK_NULL_HANDLE;
    		VkImageView           view         = VK_NULL_HANDLE;
    		VkFormat              format;
    		uint32_t              width, height, depth;
    		uint32_t              mipLevels;
    		int numChannels;
    	} texture;
    	void SetImageFormat(VkFormat  f,int n)
    	{
    	    texture.format=f;
    	    texture.numChannels=n;
    	};
    	void GeneratePerlin (uint8_t *data);
    	void SetSynthetic(bool b){
    	    m_synthesize=b;
    	}
    void prepareNoiseTexture(pv2::Context context, uint32_t width, uint32_t height, uint32_t depth, uint8_t** pixel2=nullptr);
    void updateNoiseTexture(pv2::Context context,uint8_t** pixel2=nullptr);
    void CheckImageSupport(pv2::Context context, VkFormat format, int width, int height, int depth);
    //For final MPI processing
    
    void GenerateTexture(uint8_t* data, uint8_t** images);
    void GenerateSlices(int world_size);
    void CreateMPIDescriptorSetLayout(pv2::Context context);
    void CreateMPIGraphicsPipeline(pv2::Context context, pv2::RenderBase ren);
        
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
    

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
   
    
   void SetCommandPool(VkCommandPool& p){
       m_commandPool=p;
   }
   
   std::vector<TexVertex> vertices;
   std::vector<uint16_t> indices;
   
    private:
    std::string vertMPIShaderName = "shaders/26_shader_textures.vert.spv";
    std::string fragMPIShaderName = "shaders/26_shader_textures.frag.spv";
    bool m_synthesize;
    
};




#endif //