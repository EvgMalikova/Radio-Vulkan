
#ifndef DATA
#define DATA
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

/* The main folder with all data*/

//static char* data_dir = "../data";
// static char*
// data_dir="/group/director2185/emalikova/particle-based-vis/data";

struct Aabb {
    glm::vec3 minimum;
    glm::vec3 maximum;
};

struct Vertex {
    glm::vec4 pos;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct SpaceObj {
    float position[4] = { 0, 0, 0, 0 };
    float color[4] = { 0, 0, 0, 0 };
    // float r;
    SpaceObj(float p1, float p2, float p3, float r1, float c1, float c2,
        float c3, float c4)
    {
        position[0] = p1;
        color[0] = c1;
        position[1] = p2;
        color[1] = c2;
        position[2] = p3;
        color[2] = c3;
        position[3] = r1;
        color[3] = c4;
        //  this->r=r1;
    };
    SpaceObj() {};
};

struct Particle {
    glm::vec4 pos; // xyz = position, w = mass
    glm::vec4 vel; // xyz = velocity, w = gradient texture position
};

// For nvvk::make
// For nvvk::make

struct particle {
    float x, y, z, r;
    particle(float x1, float y1, float z1, float r1)
    {
        x = x1;
        y = y1;
        z = z1;
        r = r1;
    };
    particle() { }
};

#endif //