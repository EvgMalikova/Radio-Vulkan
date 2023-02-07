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
void SimpCamera::UpdateViewMatrix()
{
	/*
    		{
    			// Look at
    			cameraLookAt=glm::normalize(cameraLookAt);
    
    			// Right vector
    			cameraRightVector = glm::cross(cameraUpVector, cameraLookAt);
    			cameraRightVector=glm::normalize(cameraRightVector);
    			
    
    			// Up vector
    			cameraUpVector = glm::cross(cameraLookAt, cameraRightVector);
    			cameraUpVector=glm::normalize(cameraUpVector);
    		}
    
	    // Build the view matrix itself
	    		cameraViewMatrix[0][0] = cameraRightVector.x;
	    		cameraViewMatrix[1][0] = cameraRightVector.y;
	    		cameraViewMatrix[2][0] = cameraRightVector.z;
	    		cameraViewMatrix[3][0] = -glm::dot(cameraRightVector, cameraPosition);
	    
	    		cameraViewMatrix[0][1] =  cameraUpVector.x;
	    		cameraViewMatrix[1][1] =  cameraUpVector.y;
	    		cameraViewMatrix[2][1] =  cameraUpVector.z;
	    		cameraViewMatrix[3][1] = -glm::dot(cameraUpVector, cameraPosition);
	    
	    		cameraViewMatrix[0][2] = cameraLookAt.x;
	    		cameraViewMatrix[1][2] = cameraLookAt.y;
	    		cameraViewMatrix[2][2] = cameraLookAt.z;
	    		cameraViewMatrix[3][2] = -glm::dot(cameraLookAt, cameraPosition);
	    
	    		cameraViewMatrix[0][3] = 0;
	    		cameraViewMatrix[1][3] = 0;
	    		cameraViewMatrix[2][3] = 0;
	    		cameraViewMatrix[3][3] = 1;
	    */
	// Generate view matrix using the eye, lookAt and up vector
	//Alternatively just
	cameraViewMatrix = glm::lookAt(GetEye(), GetTarget(), GetUpVector());

	//update vectors
	cameraRightVector = glm::transpose(cameraViewMatrix)[0];
	cameraLookAt = -glm::transpose(cameraViewMatrix)[2];
	cameraUpVector = glm::transpose(cameraViewMatrix)[1];
}

void SimpCamera::rotateAroundTarget(int width, int height, float rotate_x, float rotate_y)
{
	// Get the homogenous position of the camera and pivot point
	glm::vec4 position(GetEye().x, GetEye().y, GetEye().z, 1);
	glm::vec4 pivot(GetTarget().x, GetTarget().y, GetTarget().z, 1);

	//Amount of rotation given the mouse movement.
	//currently width dependency not used
	float deltaAngleX = (2 * M_PI / width); // a movement from left to right = 2*PI = 360 deg
	float deltaAngleY = (M_PI / height);	// a movement from top to bottom = PI = 180 deg
	float xAngle = glm::radians(rotate_x);
	float yAngle = glm::radians(rotate_y);

	// Extra step to handle the problem when the camera direction is the same as the up vector
	/*float cosAngle = glm::dot(GetViewDir(), GetUpVector());
    if (cosAngle * glm::sign(deltaAngleY) > 0.99f)
        deltaAngleY = 0.0f;*/

	// step 2: Rotate the camera around the pivot point on the first axis.
	glm::mat4x4 rotationMatrixX(1.0f);
	rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, GetUpVector());
	position = (rotationMatrixX * (position - pivot)) + pivot;

	// step 3: Rotate the camera around the pivot point on the second axis.
	glm::mat4x4 rotationMatrixY(1.0f);
	rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, GetRightVector());
	glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

	// Update the camera view (we keep the same lookat and the same up vector)
	SetEye(finalPosition);
	UpdateViewMatrix(); /**/
	//SetCameraView(finalPosition, GetLookAt(), GetUpVector());
}

void SimpCamera::rotateTargetAround(float rotate_x, float rotate_y)
{
	// Get the homogenous position of the camera and pivot point
	glm::vec4 pivot(GetEye().x, GetEye().y, GetEye().z, 1);
	glm::vec4 position(GetTarget().x, GetTarget().y, GetTarget().z, 1);

	//Amount of rotation given the mouse movement.
	float xAngle = glm::radians(rotate_x);
	float yAngle = glm::radians(rotate_y);

	// Extra step to handle the problem when the camera direction is the same as the up vector
	/*float cosAngle = glm::dot(GetViewDir(), GetUpVector());
    if (cosAngle * glm::sign(deltaAngleY) > 0.99f)
        deltaAngleY = 0.0f;*/

	// step 2: Rotate the camera around the pivot point on the first axis.
	glm::mat4x4 rotationMatrixX(1.0f);
	rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, GetUpVector());
	position = (rotationMatrixX * (position - pivot)) + pivot;

	// step 3: Rotate the camera around the pivot point on the second axis.
	glm::mat4x4 rotationMatrixY(1.0f);
	rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, GetRightVector());
	glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

	// Update the camera view (we keep the same lookat and the same up vector)
	SetTarget(finalPosition);
	UpdateViewMatrix(); /**/
	//SetCameraView(finalPosition, GetLookAt(), GetUpVector());
}

glm::vec3 SimpCamera::GetCameraPosition()
{
	return cameraPosition;
}

glm::vec3 SimpCamera::GetUpVector()
{
	return cameraUpVector;
}
glm::vec3 SimpCamera::GetTarget()
{
	return cameraTarget;
}
void SimpCamera::SetLookAt(glm::vec3 target)
{
	// Calculate relative direction from the current position
	cameraLookAt = target - cameraPosition;
	cameraLookAt = glm::normalize(cameraLookAt);

	// Normalize the right and up vectors

	cameraRightVector = glm::cross(cameraUpVector, cameraLookAt);
	cameraRightVector = glm::normalize(cameraRightVector);

	cameraUpVector = glm::cross(cameraLookAt, cameraRightVector);
	cameraRightVector = glm::normalize(cameraUpVector);

	// Construct a view matrix (special case)
	//ConstructViewMatrix(false);

	return;
}

void SimpCamera::CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, int size)
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(size);
	m_uniformBuffersMemory.resize(size);

	for (size_t i = 0; i < size; i++)
	{
		pv::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
	}
}

void SimpCamera::moveCamAndTargetForward(float distance) //
{
	// Transform movement relative to local axes
	cameraPosition += cameraLookAt * distance;
	cameraTarget += cameraLookAt * distance;
}
void SimpCamera::moveCamAndTargetUpward(float distance)
{
	// Transform movement relative to local axes
	cameraTarget += cameraUpVector * distance;
}
void SimpCamera::moveCamAndTargetRight(float distance)
{
	// Transform movement relative to local axes
	cameraTarget += cameraRightVector * distance;
}

void SimpCamera::moveForward(float distance)
{
	// Transform movement relative to local axes
	cameraPosition += cameraLookAt * distance;
	return;
}
void SimpCamera::moveUpward(float distance)
{
	// Transform movement relative to local axes
	cameraPosition += cameraUpVector * distance;
	//std::cout<<"Camera UP "<<distance<<std::endl;
}
void SimpCamera::moveRight(float distance)
{
	// Transform movement relative to local axes
	cameraPosition += cameraRightVector * distance;
}

void SimpCamera::moveTargetForward(float distance)
{
	// Transform movement relative to local axes
	cameraTarget += cameraLookAt * distance;
}
void SimpCamera::moveTargetUpward(float distance)
{
	// Transform movement relative to local axes
	cameraTarget += cameraUpVector * distance;
	return;
}
void SimpCamera::moveTargetRight(float distance)
{
	// Transform movement relative to local axes
	cameraTarget += cameraRightVector * distance;
	return;
}

void SimpCamera::UpdateUniformBuffer(VkDevice device, uint32_t currentImage, float model_scale, int width, int height, float rotate_x, float rotate_y)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	float sc = std::abs(model_scale) - 1.0;
	sc /= 10;
	float model_scale2 = sgn<float>(model_scale) * 1 + sc;
	ubo.model = glm::scale(glm::mat4(1.0), glm::vec3(model_scale2, model_scale2, 1.0)); //glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	//rotate(width, height, rotate_x, rotate_y);
	ubo.view = GetViewMatrix(); //essencial for further overwriting

	ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 250.0f);

	void *data;
	vkMapMemory(device, m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, m_uniformBuffersMemory[currentImage]);
}

void SimpCamera::UpdateUniformBuffer()
{
	uint32_t currentImage = 0;
	UpdateUniformBuffer(m_device, currentImage, m_model_scale, m_width, m_height, 0, 0);
}