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
 */

#ifndef SPLOTCH_PREVIEWER_LIBS_CORE_CAMERA
#define SPLOTCH_PREVIEWER_LIBS_CORE_CAMERA

#include <glm/glm.hpp>
#include <vector>

#include "camera.hpp"
#define GLM_FORCE_RADIANS
//#include "cxxsupport/vec3.h"
//#include "server/matrix4.h"

// // Event includes
// #include "Event.h"
// #include "../events/OnButtonPressEvent.h"
// #include "../events/OnButtonReleaseEvent.h"
// #include "../events/OnKeyPressEvent.h"
// #include "../events/OnKeyReleaseEvent.h"

// Provides the functionality within the renderers for a movable
// and configurable camera through the use of event subscriptions.
class Camera2 : public SimpCamera
{
public:
	Camera2() = default;
	Camera2(glm::vec3 eye, glm::vec3 lookat, glm::vec3 upVector)

	{
		Create(eye, lookat, upVector);
	};

	// Setters

	void SetFieldOfView(float);
	void SetMaxAcceleration(glm::vec3);
	void SetMaxSpeed(glm::vec3);

	// Creators
	void Create(glm::vec3, glm::vec3);
	void Create(glm::vec3, glm::vec3, glm::vec3);

	void RotateAroundTarget(float yaw, float pitch, float roll)
	{
		rotateAroundTarget(yaw, pitch);
		UpdateUniformBuffer();
	};
	void RotateTargetAround(float yaw, float pitch, float roll)
	{
		rotateTargetAround(yaw, pitch);
		UpdateUniformBuffer();
	};

	// Camera and target
	void MoveCamAndTargetForward(float dist)
	{
		moveCamAndTargetForward(dist);
		UpdateUniformBuffer();
	};
	void MoveCamAndTargetUpward(float dist)
	{
		MoveCamAndTargetUpward(dist);
		UpdateUniformBuffer();
	};
	void MoveCamAndTargetRight(float dist)
	{
		moveCamAndTargetRight(dist);
		UpdateUniformBuffer();
	};

	// Camera only
	void MoveForward(float dist)
	{
		moveForward(dist);
		UpdateUniformBuffer();
	};
	void MoveUpward(float dist)
	{
		moveUpward(dist);
		UpdateUniformBuffer();
	};
	void MoveRight(float dist)
	{
		moveRight(dist);
		UpdateUniformBuffer();
	};

	// Makes the camera look at a bounding box
	//void SetTarget(glm::vec3);

	// Rotation functionality - all do the same thing or as they say
	/*void Rotate(glm::vec3);
	void Rotate(float, float, float);
	void RotateYaw(float);
	void RotatePitch(float);
	void RotateRoll(float);
	void RotateAroundTarget(float, float, float);
	void RotateTargetAround(float, float, float);
	// Move functions relative to world axes 
	void Move(glm::vec3);
	void Move(float, float, float);

	// Move functions relative to local axes
	// Camera only
	void MoveForward(float);
	void MoveUpward(float);
	void MoveRight(float);
	// Camera and target
	void MoveCamAndTargetForward(float);
	void MoveCamAndTargetUpward(float);
	void MoveCamAndTargetRight(float);
	// Target only
	void MoveTargetForward(float);
	void MoveTargetUpward(float);
	void MoveTargetRight(float);

	// Physical movement
	void AccelForward(float amount);
	void AccelUpward(float amount);
	void AccelRight(float amount);
	void DecelForward(float amount);
	void DecelUpward(float amount);
	void DecelRight(float amount);
	void UpdateSpeed();
	void UpdateVelocity();
	void UpdatePosition();


	// Projections
	void SetPerspectiveProjection(float, float, float, float);
	void SetOrthographicProjection(float, float, float, float, float, float);

	// Give camera focus
	void SetFocusState(bool);

	//Matrix access
	glm::mat4 GetMVPMatrix();
	//glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
	
	//overwrite to get correct values for buffers
*/

private:
	// Proper movement for smooth camera transitions
	// Acceleration and speed are treated seperately for x y z,
	// corresponding to right, up, forward respectively
	glm::vec3 acceleration;
	glm::vec3 maxAcceleration;
	glm::vec3 velocity;
	glm::vec3 speed;
	glm::vec3 maxSpeed;

	// Camera projection settings
	float cameraFieldOfView;
	float cameraAspectRatio;
	float cameraNearClip;
	float cameraFarClip;

	// Cameras view and projection matrices

	glm::mat4 cameraProjectionMatrix;

	// Internal functions for creating view matrix
	void ConstructViewMatrix(bool);

	bool hasFocus;
};

#endif