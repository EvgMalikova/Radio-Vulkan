/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 		Tim Dykes and Ian Cant
 * 		University of Portsmouth
 *
 */\

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

	glm::vec3 Camera2::GetCameraPosition()
	{
		return cameraPosition;
	}
	glm::vec3 Camera2::GetLookAt()
	{
		return cameraLookAt;
	}
	glm::vec3 Camera2::GetUpVector()
	{
		return cameraUpVector;
	}
	glm::vec3 Camera2::GetTarget()
	{
		return cameraTarget;
	}
	void Camera2::SetLookAt(glm::vec3 target)
	{
		// Calculate relative direction from the current position
		cameraLookAt =  target - cameraPosition;
		cameraLookAt=glm::normalize(cameraLookAt);

		// Normalize the right and up vectors
		
		cameraRightVector = glm::cross(cameraUpVector, cameraLookAt);
		cameraRightVector=glm::normalize(cameraRightVector);

		cameraUpVector = glm::cross(cameraLookAt, cameraRightVector);
		cameraRightVector=glm::normalize(cameraUpVector);

		// Construct a view matrix (special case)
		ConstructViewMatrix(false);
		return;
	}

	void Camera2::SetFieldOfView(float fov)
	{
		cameraFieldOfView = fov;
		return;
	}
	
	void Camera2::SetMaxAcceleration(glm::vec3 ma)
	{
		maxAcceleration = ma;
	}

	void Camera2::SetMaxSpeed(glm::vec3 ms)
	{
		maxSpeed = ms;
	}


	// void Camera2::Create(BoundingBox box)
	// {
	// 	// Look at the bounding box
	// 	LookAtBox(box, FRONT);
	// 	return;
	// }

	void Camera2::Create(glm::vec3 position, glm::vec3 lookAt)
	{
		// Call the more useful overloaded function
		Create(position, lookAt, glm::vec3(0, 0, 1));
		return;
	}

	void Camera2::Create(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up)
	{
		// Set cameras position, up vector and fov (no modifications needed here)
		cameraPosition = position;
		cameraUpVector = up;
		velocity = glm::vec3(0,0,0);

		// Set the look at point for the camera (the cameras right vector is calculated here!)
		SetTarget(lookAt);
		return;
	}

	// void Camera2::LookAtBox(BoundingBox box)
	// {
	// 	// Call the more useful overloaded function
	// 	LookAtBox(box, FRONT);
	// 	return;
	// }

	// void Camera2::LookAtBox(BoundingBox box, int face)
	// {
	// 	// Find the correct camera position to look at the box given the prescribed
	// 	// field of view
	// 	switch(face)
	// 	{
	// 		case FRONT:
	// 		{
	// 			float distanceWidth = (((box.maxX - box.minX)/2)) / (tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxY;
	// 			float distanceHeight = (((box.maxZ - box.minZ)/2)) / (tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxY;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = (box.maxX + box.minX)/2;
	// 			cameraPosition.y = distanceFromBox;
	// 			cameraPosition.z = (box.maxZ + box.minZ)/2;

	// 			cameraUpVector = glm::vec3(0, 0, 1);
	// 			break;
	// 		}

	// 		case LEFT:
	// 		{
	// 			float distanceWidth = (((box.maxY - box.minY)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxX;
	// 			float distanceHeight = (((box.maxZ - box.minZ)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxX;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = distanceFromBox;
	// 			cameraPosition.y = (box.maxY + box.minY)/2;
	// 			cameraPosition.z = (box.maxZ + box.minZ)/2;

	// 			cameraUpVector = glm::vec3(0, 0, 1);
	// 			break;
	// 		}

	// 		case RIGHT:
	// 		{
	// 			float distanceWidth = (((box.maxY - box.minY)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxX;
	// 			float distanceHeight = (((box.maxZ - box.minZ)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxX;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = -distanceFromBox;
	// 			cameraPosition.y = (box.maxY + box.minY)/2;
	// 			cameraPosition.z = (box.maxZ + box.minZ)/2;

	// 			cameraUpVector = glm::vec3(0, 0, 1);
	// 			break;
	// 		}

	// 		case BACK:
	// 		{
	// 			float distanceWidth = (((box.maxX - box.minX)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxY;
	// 			float distanceHeight = (((box.maxZ - box.minZ)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxY;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = (box.maxX + box.minX)/2;
	// 			cameraPosition.y = -distanceFromBox;
	// 			cameraPosition.z = (box.maxZ + box.minZ)/2;

	// 			cameraUpVector = glm::vec3(0, 0, 1);
	// 			break;
	// 		}

	// 		case TOP:
	// 		{
	// 			float distanceWidth = (((box.maxX - box.minX)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxZ;
	// 			float distanceHeight = (((box.maxY - box.minY)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxZ;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = (box.maxX + box.minX)/2;
	// 			cameraPosition.y = (box.maxY + box.minY)/2;
	// 			cameraPosition.z = distanceFromBox;

	// 			cameraUpVector = glm::vec3(0, 1, 0);
	// 			break;
	// 		}

	// 		case BOTTOM:
	// 		{
	// 			float distanceWidth = (((box.maxX - box.minX)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxZ;
	// 			float distanceHeight = (((box.maxY - box.minY)/2) / tan(Math::degreesToRadians(cameraFieldOfView/2))) + box.maxZ;

	// 			float distanceFromBox = (distanceWidth > distanceHeight) ? distanceWidth : distanceHeight;

	// 			cameraPosition.x = (box.maxX + box.minX)/2;
	// 			cameraPosition.y = (box.maxY + box.minY)/2;
	// 			cameraPosition.z = -distanceFromBox;

	// 			cameraUpVector = glm::vec3(0, 1, 0);
	// 			break;
	// 		}
	// 	}

	// 	// Set the look at point for the camera (the cameras right vector is calculated here)
	// 	SetTarget(box.centerPoint);
	// 	return;
	// }

	void Camera2::SetTarget(glm::vec3 inTarget)
	{
		cameraTarget = inTarget;
		SetLookAt(cameraTarget);
	}

	void Camera2::Rotate(glm::vec3 rotation)
	{
		// Call overloaded member
		Rotate(rotation.x, rotation.y, rotation.z);
		return;
	}
	void Camera2::Rotate(float yaw, float pitch, float roll)
	{
		// Create a rotation matrix
		glm::mat4 rotationMatrix=glm::mat4(1.0f);
		

		if(yaw != 0.0f)
		{
			// Create the yaw component in the roation matrix
			rotationMatrix=glm::rotate(rotationMatrix, yaw,cameraUpVector);

			// Transform the right and look at vector values
			cameraRightVector = glm::vec3( glm::vec4(cameraRightVector,1) * rotationMatrix);
			cameraLookAt = glm::vec3( glm::vec4(cameraLookAt,1) * rotationMatrix);
		}

		if(pitch != 0.0f)
		{
			// Create the pitch component in the rotation matrix
			rotationMatrix=glm::rotate(rotationMatrix,  pitch,cameraRightVector);

			// Transform the up and look at vector values
			cameraUpVector = glm::vec3( glm::vec4(cameraUpVector,1) * rotationMatrix);
			cameraLookAt = glm::vec3( glm::vec4(cameraLookAt,1) * rotationMatrix);
		}

		if(roll != 0.0f)
		{
			// Create the roll component in the rotation matrix
			rotationMatrix=glm::rotate(rotationMatrix, roll,cameraLookAt);

			// Transform the right and up vector values
			cameraUpVector = glm::vec3( glm::vec4(cameraUpVector,1) * rotationMatrix);
			cameraRightVector = glm::vec3( glm::vec4(cameraRightVector,1) * rotationMatrix);
		}

		return;
	}
	void Camera2::RotateYaw(float yaw)
	{
		// Create a rotation matrix
		glm::mat4 rotationMatrix=glm::mat4(1.0f);
		

		if(yaw != 0.0f)
		{
			// Create the yaw component in the roation matrix
			rotationMatrix=glm::rotate(rotationMatrix, yaw, cameraUpVector);

			// Transform the right and look at vector values
			cameraRightVector = glm::vec3( glm::vec4(cameraRightVector,1) * rotationMatrix);
			cameraLookAt = glm::vec3( glm::vec4(cameraLookAt,1) * rotationMatrix);
		}

		return;
	}
	void Camera2::RotatePitch(float pitch)
	{
		// Create a rotation matrix
		glm::mat4 rotationMatrix=glm::mat4(1.0f);
		

		if(pitch != 0.0f)
		{
			// Create the pitch component in the rotation matrix
			rotationMatrix=glm::rotate(rotationMatrix, pitch, cameraRightVector);
			

			// Transform the up and look at vector values
			cameraUpVector =glm::vec3( glm::vec4(cameraUpVector,1) * rotationMatrix);
			cameraLookAt = glm::vec3( glm::vec4(cameraLookAt,1) * rotationMatrix);
		}

		return;
	}
	void Camera2::RotateRoll(float roll)
	{
		// Create a rotation matrix
		glm::mat4 rotationMatrix=glm::mat4(1.0f);

		if(roll != 0.0f)
		{
			// Create the roll component in the rotation matrix
			rotationMatrix=glm::rotate(rotationMatrix, roll, cameraLookAt);
						
			

			// Transform the right and up vector values
			cameraUpVector =glm::vec3( glm::vec4(cameraUpVector,1) * rotationMatrix);
			cameraRightVector = glm::vec3(glm::vec4(cameraRightVector,1) * rotationMatrix);
		}

		return;
	}

	void Camera2::RotateAroundTarget(float yaw, float pitch, float roll)
	{
		glm::mat4 rotationMtxH=glm::mat4(1.0f);
		glm::mat4 rotationMtxP=glm::mat4(1.0f);
		
		glm::vec4 focusVector = glm::vec4(cameraTarget - cameraPosition,1);

		

		if(yaw != 0.0f)
		{
			rotationMtxH=glm::rotate(rotationMtxH, yaw, cameraUpVector);
			
			focusVector = focusVector * rotationMtxH;
		}

		if(pitch != 0.0f)
		{   rotationMtxP=glm::rotate(rotationMtxP, pitch, cameraRightVector);
			
			focusVector = focusVector * rotationMtxP;
		}

		if(roll != 0.0f)
		{
			// Ignore roll for now
		}

		cameraPosition = glm::vec3(focusVector) + cameraTarget;

		SetLookAt(cameraTarget);
	}

	void Camera2::RotateTargetAround(float yaw, float pitch, float roll)
	{
		glm::mat4 rotationMtxH=glm::mat4(1.0f);
		glm::mat4 rotationMtxP=glm::mat4(1.0f);

		glm::vec4 focusVector = glm::vec4(cameraTarget - cameraPosition,1);

		if(yaw != 0.0f)
		{
			rotationMtxH=glm::rotate(glm::mat4(1.0f), yaw, cameraUpVector);
			//rotationMtxH.rotate(cameraUpVector, yaw);
			focusVector = rotationMtxH*focusVector;
		}

		if(pitch != 0.0f)
		{
			rotationMtxP=glm::rotate(rotationMtxP, pitch, cameraRightVector);
						
			//rotationMtxP.rotate(cameraRightVector, pitch);
			focusVector = focusVector * rotationMtxP;
		}

		if(roll != 0.0f)
		{
			// Ignore roll for now
		}

		cameraTarget = glm::vec3(focusVector) + cameraPosition;

		SetLookAt(cameraTarget);
	}

	void Camera2::Move(glm::vec3 movement)
	{
		// Move the camera relative to world axes
		cameraPosition += movement;
		return;
	}
	void Camera2::Move(float x, float y, float z)
	{
		// Move the camera relative to world axes
		cameraPosition.x += x;
		cameraPosition.y += y;
		cameraPosition.z += z;
		return;
	}
	void Camera2::MoveForward(float distance)
	{
		// Transform movement relative to local axes 
		cameraPosition += cameraLookAt * distance;
		return;
	}
	void Camera2::MoveUpward(float distance)
	{
		// Transform movement relative to local axes 
		cameraPosition += cameraUpVector * distance;
		return;
	}
	void Camera2::MoveRight(float distance)
	{
		// Transform movement relative to local axes 
		cameraPosition += cameraRightVector * distance;
		return;
	}

	void Camera2::MoveCamAndTargetForward(float distance)
	{
		// Transform movement relative to local axes 
		cameraPosition += cameraLookAt * distance;
		cameraTarget += cameraLookAt * distance;
		return;
	}
	void Camera2::MoveCamAndTargetUpward(float distance)
	{
		// Transform movement relative to local axes 
		cameraTarget += cameraUpVector * distance;
		return;
	}
	void Camera2::MoveCamAndTargetRight(float distance)
	{
		// Transform movement relative to local axes 
		cameraTarget += cameraRightVector * distance;
		return;
	}

	void Camera2::MoveTargetForward(float distance)
	{
		// Transform movement relative to local axes 
		cameraTarget += cameraLookAt * distance;
		return;
	}
	void Camera2::MoveTargetUpward(float distance)
	{
		// Transform movement relative to local axes 
		cameraTarget += cameraUpVector * distance;
		return;
	}
	void Camera2::MoveTargetRight(float distance)
	{
		// Transform movement relative to local axes 
		cameraTarget += cameraRightVector * distance;
		return;
	}

	void Camera2::AccelForward(float amount)
	{
		// Transform movement relative to local axes 
		acceleration.z += amount;
		if(acceleration.z > maxAcceleration.z)
			acceleration.z = maxAcceleration.z;
		return;
	}

	void Camera2::AccelUpward(float amount)
	{
		// Transform movement relative to local axes 
		acceleration.y += amount;
		if(acceleration.y > maxAcceleration.y)
			acceleration.y = maxAcceleration.y;
		return;
	}

	void Camera2::DecelForward(float amount)
	{
		// z *= 0.x
	}

	void Camera2::DecelUpward(float amount)
	{

	}

	void Camera2::DecelRight(float amount)
	{

	}

	void Camera2::AccelRight(float amount)
	{
		// Transform movement relative to local axes 
		acceleration.x += amount;
		if(acceleration.x > maxAcceleration.x)
			acceleration.x = maxAcceleration.x;
		return;
	}

	void Camera2::UpdateSpeed()
	{
		speed += acceleration;
		if(speed.x > maxSpeed.x) speed.x = maxSpeed.x;
		if(speed.y > maxSpeed.y) speed.y = maxSpeed.y;
		if(speed.z > maxSpeed.z) speed.z = maxSpeed.z;
	}

	void Camera2::UpdateVelocity()
	{
		velocity = (cameraLookAt * speed.z) + (cameraUpVector * speed.y) + (cameraRightVector * speed.x);
	}

	void Camera2::UpdatePosition()
	{
		cameraPosition += velocity;
	}

	void Camera2::SetFocusState(bool newFocus)
	{
		hasFocus = newFocus;
	}

	void Camera2::SetPerspectiveProjection(float fov, float aspectRatio, float nearClip, float farClip)
	{
		SetFieldOfView(fov);
		// Create and set camera's projection matrix (perspective) 
		cameraProjectionMatrix = glm::perspective(fov, aspectRatio, nearClip, farClip);
	}

	void Camera2::SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
	{
		// Create and set camera's projection matrix (orthgraphic) 
		cameraProjectionMatrix = glm::ortho(left, right, bottom, top, near, far);
	}

	glm::mat4 Camera2::GetMVPMatrix()
	{
		
		// Rebuild the view matrix
		ConstructViewMatrix(true);

		// Create and return ModelViewProjection Matrix
		return (cameraViewMatrix*cameraProjectionMatrix);
	}

	glm::mat4 Camera2::GetViewMatrix()
	{
		// Rebuild and return the view matrix
		ConstructViewMatrix(true);

		return cameraViewMatrix;
	}

	glm::mat4 Camera2::GetProjectionMatrix()
	{
		return cameraProjectionMatrix;
	}


	void Camera2::ConstructViewMatrix(bool orthoganalizeAxes)
	{
		// Orthogonalize axes if necessary (ensure up and right vectors are perpendicular to lookat and each other)
		if(orthoganalizeAxes)
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


		// std::cout << "right.x: " << cameraRightVector.x << std::endl;
		// std::cout << "right.y: " << cameraRightVector.y << std::endl;
		// std::cout << "right.z: " << cameraRightVector.z << std::endl;
		// std::cout << "up.x: " << cameraUpVector.x << std::endl;
		// std::cout << "up.y: " << cameraUpVector.y << std::endl;
		// std::cout << "up.z: " << cameraUpVector.z << std::endl;
		// std::cout << "at.x: " << cameraLookAt.x << std::endl;
		// std::cout << "at.y: " << cameraLookAt.y << std::endl;
		// std::cout << "at.z: " << cameraLookAt.z << std::endl;
		// std::cout << "position.x: " << cameraPosition.x << std::endl;
		// std::cout << "position.y: " << cameraPosition.y << std::endl;
		// std::cout << "position.z: " << cameraPosition.z << std::endl;

		// std::cout << "Dotprod right and pos: " << dotprod(cameraRightVector,cameraPosition) << std::endl;
		// std::cout << "Dotprod up and pos: " << dotprod(cameraUpVector,cameraPosition) << std::endl;
		// std::cout << "Dotprod at and pos: " << dotprod(cameraLookAt,cameraPosition) << std::endl;

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

		// std::cout << "trans[0][0]: " << cameraViewMatrix[0][0] << std::endl;
		// std::cout << "trans[0][1]: " << cameraViewMatrix[0][1] << std::endl;
		// std::cout << "trans[0][2]: " << cameraViewMatrix[0][2] << std::endl;
		// std::cout << "trans[0][3]: " << cameraViewMatrix[0][3] << std::endl;
		// std::cout << "trans[1][0]: " << cameraViewMatrix[1][0] << std::endl;
		// std::cout << "trans[1][1]: " << cameraViewMatrix[1][1] << std::endl;
		// std::cout << "trans[1][2]: " << cameraViewMatrix[1][2] << std::endl;
		// std::cout << "trans[1][3]: " << cameraViewMatrix[1][3] << std::endl;
		// std::cout << "trans[2][0]: " << cameraViewMatrix[2][0] << std::endl;
		// std::cout << "trans[2][1]: " << cameraViewMatrix[2][1] << std::endl;
		// std::cout << "trans[2][2]: " << cameraViewMatrix[2][2] << std::endl;
		// std::cout << "trans[2][3]: " << cameraViewMatrix[2][3] << std::endl;
		// std::cout << "trans[3][0]: " << cameraViewMatrix[3][0] << std::endl;
		// std::cout << "trans[3][1]: " << cameraViewMatrix[3][1] << std::endl;
		// std::cout << "trans[3][2]: " << cameraViewMatrix[3][2] << std::endl;
		// std::cout << "trans[3][3]: " << cameraViewMatrix[3][3] << std::endl;

		// exit(0);
		return;
	}

