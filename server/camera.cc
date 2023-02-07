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
		//SimpCamera(position,lookAt,up);
		
		SetEye(position);
		SetTarget(lookAt);
		SetUp(up);
		UpdateViewMatrix();
		        
		/*// Set cameras position, up vector and fov (no modifications needed here)
		cameraPosition = position;
		cameraUpVector = up;
		velocity = glm::vec3(0,0,0);
		
		
		

		// Set the look at point for the camera (the cameras right vector is calculated here!)
		SetTarget(lookAt);
		
		//From SimpCamera
		ConstructViewMatrix(false);
		//m_viewMatrix=cameraViewMatrix;
		//UpdateViewMatrix(); //to be replaced with ConstructViewMatrix(false); in initial camera
		return;*/
		m_width=1000;
		m_height=1000;
		m_model_scale=1.0;
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

/*

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

	void Camera2::RotateAroundTarget(float rotate_x, float rotate_y, float roll)
	{
		
		float deltaAngleX = (2 * M_PI / 1000); // a movement from left to right = 2*PI = 360 deg
		        float deltaAngleY = (M_PI / 1000); // a movement from top to bottom = PI = 180 deg
		        float yaw = rotate_x * deltaAngleX;
		        float pitch = rotate_y * deltaAngleY;
		        
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
        SetEye(finalPosition);
		//SetLookAt(cameraTarget);
		UpdateViewMatrix();
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
		UpdateViewMatrix();
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


	

		// Build the view matrix itself
		

		UpdateViewMatrix(); 



		// exit(0);
		return;
	}

*/