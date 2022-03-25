#include "camera.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>


template <typename T>
int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

void SimpCamera::CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, int size)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(size);
    m_uniformBuffersMemory.resize(size);

    for (size_t i = 0; i < size; i++) {
        pv::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }
}

void SimpCamera::UpdateUniformBuffer(VkDevice device, uint32_t currentImage, float model_scale, int width, int height, float rotate_x, float rotate_y)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo {};
    float sc = std::abs(model_scale) - 1.0;
    sc /= 10;
    float model_scale2 = sgn<float>(model_scale) * 1 + sc;
    ubo.model = glm::scale(glm::mat4(1.0), glm::vec3(model_scale2, model_scale2, 1.0)); //glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    rotate(width, height, rotate_x, rotate_y);
    ubo.view = GetViewMatrix();

    ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 250.0f);

    void* data;
    vkMapMemory(device, m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, m_uniformBuffersMemory[currentImage]);
}