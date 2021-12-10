#include "loaders.hpp"
#include "cxxsupport/vec3.h"
#include "kernel/colour.h"
#include "kernel/colourmap.h"

//#include <limits.h>
//#include <fstream>

#include "VulkanBuffer.h"
#include "VulkanTools.h"

#include "fitsReader.h"

#include <chrono>
//#include "math.h" // for RAND, and rand
#include "context.hpp"
#include <random>

namespace pv {
double sampleNormal()
{
    double u = ((double)rand() / (RAND_MAX)) * 2 - 1;
    double v = ((double)rand() / (RAND_MAX)) * 2 - 1;
    double r = u * u + v * v;
    const double mean = 0.0;
    const double stddev = 0.1;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    r = dist(generator);
    if (r == 0 || r > 1)
        return sampleNormal();
    //double c = sqrt(-2 * log(r) / r);
    // return u * c;
    return r;
}

void PaticleLoader::CreateVertexBuffer(VkCommandPool commandPool, VkDevice device, VkPhysicalDevice phdevice, VkQueue queueGCT)
{

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pv::createBuffer(device, phdevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    pv::createBuffer(device, phdevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    pv::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize, commandPool, device, queueGCT);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/*   void PaticleLoader::GPULoadVertexBuffer(VkCommandPool commandPool, pv::pvContext context) {
    
      // std::vector<uint32_t> indices = { 0, 1, 2 };
    
      const VkDeviceSize vertexBufferSize = vertices.size() * sizeof(SpaceObj);
    
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingMemory;
    
      // Command buffer for copy commands
      VkCommandBufferAllocateInfo cmdBufAllocateInfo =
          pv::commandBufferAllocateInfo(
              commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
      VkCommandBuffer copyCmd;
      VK_CHECK_RESULT(
          vkAllocateCommandBuffers(context.m_device, &cmdBufAllocateInfo, &copyCmd));
      VkCommandBufferBeginInfo cmdBufInfo =
          pv::commandBufferBeginInfo();
    
      VkBuffer vertexBuffer;
      // Copy input data with a staging buffer
      {
        // Vertices
        pv::createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingMemory, vertexBufferSize,context,
                     vertices.data());
    
        pv::createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer,
                     &vertexMemory, vertexBufferSize,context);
    
        VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(copyCmd, stagingBuffer, vertexBuffer, 1, &copyRegion);
        VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));
    
        //vertexBuffers.push_back(vertexBuffer);
       // std::cout << queues.size() << std::endl;
        pv::submitWork(copyCmd,context.m_queueGCT,context.m_device);
    
        std::cout << "Work was submitted" << std::endl;
        vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
        vkFreeMemory(context.m_device, stagingMemory, nullptr);
      }
    }
   */

void PaticleLoader::LoadData(int world_size, int world_rank)
{
    ReadData(world_size, world_rank);
    // ReadBinary();

    std::cout << "particles " << particle_data.size() << " \n";
    std::cout << "part1 " << particle_data[0].x << "," << particle_data[0].y
              << "," << particle_data[0].z << "  " << particle_data[0].r
              << "  " << particle_data[0].I << "  " << particle_data[0].e.g
              << " \n";

    // TODO star per node block
    PrepaireVertices();
}

/*
            Read data
    */
void PaticleLoader::ReadData(int num_ranks_, int rank)
{
    std::string FileName2 = std::string(data_dir) + "/" + m_fitsFilename; //LVHIS027.na.icln.fits//"; /// cont_eval.fits";//LVHIS027.na.icln.fits";//sky_ldev_v2.fits";//sky_eval.fits";//sky_ldev_v2.fits";
        /// /cont_full.fits";/

    fitsReader fits;
    fits.rms_koef = 3.5;
    fits.is3D = true;
    fits.SetFileName(FileName2.c_str()); // LVHIS027.na.icln.fits");//
    // fits.CalculateRMS(particle_data);
    fits.m_type = this->m_type;
    fits.SetMPI(num_ranks_, rank);

    // TODO: read particle_data per MPI node
    fits.ReadFits(particle_data);

    this->max = fits.GetMax();
    this->min = fits.GetMin();
    // this->rms=fits.GetRMS();
    std::cout << "in process " << rank << "of total " << num_ranks_
              << std::endl;
    std::cout << this->min << " min " << this->max << " max" << std::endl;
    std::cout << particle_data.size() << " r= " << particle_data[0].r
              << " I= " << particle_data[0].I << std::endl;

    // const char* fileN="sky_ldev_v2.bin";
    // write_binary(fileN,particle_data);
}
void PaticleLoader::ReadBinary(int num_ranks_, int rank)
{
    std::string FileName3 = std::string(data_dir) + "/sky_Idev.par"; /// sky_eval.par";//

    paramfile params2(FileName3.c_str(), false);
    bin_reader_tab(params2, particle_data);
}

void PaticleLoader::PrepaireVertices()
{

    int num = 0;
    int skip_rate = 1;

    p_min = particle(particle_data[0].x, particle_data[0].y, particle_data[0].z,
        particle_data[0].r);
    p_max = particle(particle_data[0].x, particle_data[0].y, particle_data[0].z,
        particle_data[0].r);

    for (int i = 0; i < particle_data.size(); i++) {
        // if (particle_data[i].r > 0.0000001) //gas
        {
            particle pp = particle(particle_data[i].x, particle_data[i].y,
                particle_data[i].z, particle_data[i].r);

            num++;
            p_min.x = std::min(p_min.x, (float)pp.x);
            p_min.y = std::min(p_min.y, (float)pp.y);
            p_min.z = std::min(p_min.z, (float)pp.z);
            p_min.r = std::min(p_min.r, (float)pp.r);

            if (pp.x > p_max.x)
                p_max.x = pp.x;
            if (pp.y > p_max.y)
                p_max.y = pp.y;
            if (pp.z > p_max.z)
                p_max.z = pp.z;
            if (pp.r > p_max.r)
                p_max.r = pp.r;
        }
    }

    float x_scale = p_max.x - p_min.x;
    float y_scale = p_max.y - p_min.y;
    float z_scale = p_max.z - p_min.z;
    float r_scale = p_max.r - p_min.r;

    // TODO: scaling for fits
    /// r_scale * sc - carefull

    glm::vec3 noise = glm::vec3(sampleNormal(), sampleNormal(), sampleNormal());
    m_center = glm::vec3((p_min.x + p_max.x) / 2, (p_min.y + p_max.y) / 2,
                   (p_min.z + p_max.z) / 2)
        + noise;

    float sc = 2.0; // 10.2; //for camera in 20
    float scale = fmax(x_scale, y_scale);
    // scale=fmax(scale,z_scale);
    float zDefScale = 8;
    float sc2 = sc / zDefScale; // 4 - should be user defined scaling of flux
        // dim

    scale = sc / (scale);
    m_scale = glm::vec3(scale, scale,
        sc2 / z_scale); // normalise it all from -1,1, except
        // for third, that is additionally scaled

    std::cout << "check boundaries " << std::endl;
    std::cout << (p_min.x - m_center.x) * m_scale.x << ","
              << (p_min.y - m_center.y) * m_scale.y << ","
              << (p_min.z - m_center.z) * m_scale.z << std::endl;
    // Set Camera parameters

    z_bounds = glm::vec2(-1.0 / zDefScale, 1.0 / zDefScale);
    camDist = 1.5; // user defined distance adjustment
    cameraPos = glm::vec3(0, 0, z_bounds[0] - camDist);
    cameraTarget = glm::vec3(
        0.5, 1.0,
        0); // glm::vec3(0, 0, 0);//glm::vec3(0.25*zDefScale, 2.0*zDefScale, 0);
        // //glm::vec3(0.5, 1.0, -0.01);// //approximate coord of target to
        // look at

    for (int i = 0; i < particle_data.size(); i++) {
        COLOUR e = particle_data[i].e;
        Vertex s;
        s.pos = glm::vec4((particle_data[i].x - m_center.x) * m_scale.x,
            (particle_data[i].y - m_center.y) * m_scale.y,
            (particle_data[i].z - m_center.z) * (m_scale.z),
            1.6 * ((particle_data[i].r)));
        s.color = glm::vec4(e.r, e.g, e.b,
            0.3 * (particle_data[i].I));

        //SpaceObj s((particle_data[i].x - m_center.x) * m_scale.x,
        //           (particle_data[i].y - m_center.y) * m_scale.y,
        //           (particle_data[i].z - m_center.z) * (m_scale.z),
        //           1.0 * ((particle_data[i].r)), e.r, e.g, e.b,
        //           7.0 * (particle_data[i].I));
        vertices.push_back(s);
        vertices2.push_back((particle_data[i].x - m_center.x) * m_scale.x);
        vertices2.push_back((particle_data[i].y - m_center.y) * m_scale.y);
        vertices2.push_back((particle_data[i].z - m_center.z) * (m_scale.z));
    }

    vertices.resize(particle_data.size());
    vertices2.resize(particle_data.size() * 3);
    printf("%lu number of vertices \n", vertices.size());
    printf("%lu number of vertices \n", vertices2.size());
}

}