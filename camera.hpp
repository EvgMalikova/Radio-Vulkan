#ifndef Camera
#define Camera
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class SimpCamera
{
public:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    SimpCamera() = default;

    SimpCamera(glm::vec3 eye, glm::vec3 lookat, glm::vec3 upVector)
        : cameraPosition(std::move(eye)), cameraTarget(std::move(lookat)), cameraUpVector(std::move(upVector))
    {
        UpdateViewMatrix();
    }

    void SetWindowSize(int width, int height)
    {
        m_width = width;
        m_height = height;
    }
    void SetModelScale(float model_scale)
    {
        m_model_scale = model_scale;
    }

    void SetDevice(VkDevice dev)
    {
        m_device = dev;
    }

    // Getters

    // Camera forward is -z
    glm::vec3 GetLookAt()
    {
        return cameraLookAt;
    };

    glm::vec3 GetRightVector()
    {

        return cameraRightVector;
    }

    glm::vec3 GetCameraPosition();

    glm::vec3 GetUpVector();
    glm::vec3 GetTarget();

    void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up)
    {
        SetEye(std::move(eye));
        SetTarget(std::move(lookat));
        SetUp(std::move(up));
        UpdateViewMatrix();
    }

    //Setters
    void SetLookAt(glm::vec3 target);
    /*void SetTarget(glm::vec3 inTarget)
    	{
    		cameraTarget = inTarget;
    		SetLookAt(cameraTarget);
    	}*/

    //overwrite to get correct values for buffers
    void UpdateViewMatrix();

    glm::mat4x4 GetViewMatrix() const { return cameraViewMatrix; }
    glm::vec3 GetEye() const { return cameraPosition; }
    glm::vec3 GetUpVector() const { return cameraUpVector; }
    glm::vec3 GetTarget() const { return cameraTarget; }

    void SetEye(glm::vec3 eye) { cameraPosition = eye; };
    void SetTarget(glm::vec3 lookat) { cameraTarget = lookat; };
    void SetUp(glm::vec3 up) { cameraUpVector = up; };

    void rotateAroundTarget(float rotate_x, float rotate_y)
    {
        rotateAroundTarget(m_width, m_height, rotate_x, rotate_y);
    }

    void rotateAroundTarget(int width, int height, float rotate_x, float rotate_y);

    void rotateTargetAround(float rotate_x, float rotate_y);

    // Camera and target
    void moveCamAndTargetForward(float);
    void moveCamAndTargetUpward(float);
    void moveCamAndTargetRight(float);

    // Camera only
    void moveForward(float);
    void moveUpward(float);
    void moveRight(float);

    void moveTargetForward(float);
    void moveTargetUpward(float);
    void moveTargetRight(float);

    void CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, int size);
    void UpdateUniformBuffer();
    void UpdateUniformBuffer(VkDevice device, uint32_t currentImage, float model_scale, int width, int height, float rotate_x, float rotate_y);
    void CleanUp(VkDevice device, int i)
    {
        vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
    }

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    int m_width;
    int m_height;
    float m_model_scale;

private:
    // Camera positions and targets including orientation
    glm::vec3 cameraPosition;
    glm::vec3 cameraLookAt; // Relative to cameraPosition (normalised directional vector)
    glm::vec3 cameraTarget; // Setable target point to lookat/rotate around
    glm::vec3 cameraUpVector;
    glm::vec3 cameraRightVector;
    glm::mat4 cameraViewMatrix;

    VkDevice m_device;
};

#endif //