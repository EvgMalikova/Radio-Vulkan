#ifndef Camera
#define Camera
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class SimpCamera {
public:
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    SimpCamera() = default;

    SimpCamera(glm::vec3 eye, glm::vec3 lookat, glm::vec3 upVector)
        : m_eye(std::move(eye))
        , m_lookAt(std::move(lookat))
        , m_upVector(std::move(upVector))
    {
        UpdateViewMatrix();
    }

    glm::mat4x4 GetViewMatrix() const { return m_viewMatrix; }
    glm::vec3 GetEye() const { return m_eye; }
    glm::vec3 GetUpVector() const { return m_upVector; }
    glm::vec3 GetLookAt() const { return m_lookAt; }

    // Camera forward is -z
    glm::vec3 GetViewDir() const { return -glm::transpose(m_viewMatrix)[2]; }
    glm::vec3 GetRightVector() const { return glm::transpose(m_viewMatrix)[0]; }

    void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up)
    {
        m_eye = std::move(eye);
        m_lookAt = std::move(lookat);
        m_upVector = std::move(up);
        UpdateViewMatrix();
    }

    void UpdateViewMatrix()
    {
        // Generate view matrix using the eye, lookAt and up vector
        m_viewMatrix = glm::lookAt(m_eye, m_lookAt, m_upVector);
    }

    void rotate(int width, int height, float rotate_x, float rotate_y)
    {
        // Get the homogenous position of the camera and pivot point
        glm::vec4 position(GetEye().x, GetEye().y, GetEye().z, 1);
        glm::vec4 pivot(GetLookAt().x, GetLookAt().y, GetLookAt().z, 1);

        // step 1 : Calculate the amount of rotation given the mouse movement.
        float deltaAngleX = (2 * M_PI / width); // a movement from left to right = 2*PI = 360 deg
        float deltaAngleY = (M_PI / height); // a movement from top to bottom = PI = 180 deg
        float xAngle = rotate_x * deltaAngleX;
        float yAngle = rotate_y * deltaAngleY;

        // Extra step to handle the problem when the camera direction is the same as the up vector
        float cosAngle = glm::dot(GetViewDir(), m_upVector);
        if (cosAngle * glm::sign(deltaAngleY) > 0.99f)
            deltaAngleY = 0.0f;

        // step 2: Rotate the camera around the pivot point on the first axis.
        glm::mat4x4 rotationMatrixX(1.0f);
        rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, m_upVector);
        position = (rotationMatrixX * (position - pivot)) + pivot;

        // step 3: Rotate the camera around the pivot point on the second axis.
        glm::mat4x4 rotationMatrixY(1.0f);
        rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, GetRightVector());
        glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

        // Update the camera view (we keep the same lookat and the same up vector)
        SetCameraView(finalPosition, GetLookAt(), m_upVector);
    }

    //------------------------------------------------------------------------------
    void rotateH(float s, bool bPan = false)
    {
        glm::vec3 p = m_eye;
        glm::vec3 o = m_lookAt;
        glm::vec3 po = p - o;
        float l = glm::length(po);
        po = glm::normalize(po);
        glm::vec3 dv = glm::cross(po, glm::vec3(0, 1, 0));
        dv *= s;
        p += dv;
        po = p - o;
        float l2 = glm::length(po);
        po = glm::normalize(po);
        l = l2 - l;
        p -= (l / l2) * (po);
        m_eye = p;
        if (bPan)
            m_lookAt += dv;
    }
    //------------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------------
    void rotateV(float s, bool bPan = false)
    {
        glm::vec3 p = m_eye;
        glm::vec3 o = m_lookAt;
        glm::vec3 po = p - o;
        float l = glm::length(po);
        po = glm::normalize(po);
        glm::vec3 dv = glm::cross(po, glm::vec3(0, -1, 0));
        dv = glm::normalize(dv);
        glm::vec3 dv2 = glm::cross(po, dv);
        dv2 *= s;
        p += dv2;
        po = p - o;
        float l2 = glm::length(po);
        po = glm::normalize(po);

        if (bPan)
            m_lookAt += dv2;

        // protect against gimbal lock
        if (std::fabs(glm::dot(po / l2, glm::vec3(0, 1, 0))) > 0.99)
            return;

        l = l2 - l;
        p -= (l / l2) * (po);
        m_eye = p;
    }

    void CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, int size);
    void UpdateUniformBuffer(VkDevice device, uint32_t currentImage, float model_scale, int width, int height, float rotate_x, float rotate_y);
    void CleanUp(VkDevice device, int i)
    {
        vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
    }

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

private:
    glm::mat4x4 m_viewMatrix;
    glm::vec3 m_eye; // Camera position in 3D
    glm::vec3 m_lookAt; // Point that the camera is looking at
    glm::vec3 m_upVector; // Orientation of the camera
};

#endif //