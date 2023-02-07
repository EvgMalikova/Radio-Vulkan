#ifndef Loader
#define Loader

#include "dataStruct.h"
#include "helpers.hpp"
#include "reader/reader.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace pv {

class PaticleLoader {
public:
    
    struct Aabbs     // Buffer of all Aabb  vks::Buffer
    		{
    			int count;
    			VkBuffer buffer;
    			VkDeviceMemory memory;
    		} aabbs;
    void CreateAABBsBuffer(VkCommandPool commandPool, VkDevice device, VkPhysicalDevice phdevice, VkQueue queueGCT);
    		
    /*Data reading */
    void LoadData(int world_size, int world_rank);
    //#ifdef USE_NVVK
    // void GPULoadVertexBuffer(VkCommandPool commandPool, nvvk::Context context);
    // #else
    //void GPULoadVertexBuffer(VkCommandPool commandPool, pv::pvContext context);
    // #endif;
    void CreateVertexBuffer(VkCommandPool commandPool, VkDevice device, VkPhysicalDevice phdevice, VkQueue queueGCT);

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    PaticleLoader() { m_type = 0; };
    ~PaticleLoader() {};

    std::vector<Vertex> vertices;
    std::vector<float> vertices2;

    void SetFileName(std::string filename)
    {
        m_fitsFilename = filename;
    }
    void SetDataDir(std::string dir)
    {
        data_dir = dir;
    }

    void CleanUp(VkDevice device)
    {
        vkDestroyBuffer(device, m_vertexBuffer, nullptr);
        vkFreeMemory(device, m_vertexBufferMemory, nullptr);
    }

private:
    void ReadData(int num_ranks_, int rank);
    void ReadBinary(int num_ranks_, int rank);
    void PrepaireVertices();

    std::vector<particle_sim> particle_data;
    float min;
    float max;
    int type;
    float minBound[3];
    float maxBound[3];

    uint32_t numParticles;
    particle p_min;
    particle p_max;
    glm::vec3 m_center;
    glm::vec3 m_scale;
    std::string data_dir;

    int m_type;

    /*Camera parameters to setup*/
    // Camera parameters
    glm::vec2 z_bounds;
    float camDist; // user defined distance adjustment
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;

    VkDeviceMemory vertexMemory;

    std::string m_fitsFilename;
};
}
#endif //
