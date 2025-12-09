#pragma once

#include "math/Vector.h"

#include <PxPhysics.h>
#include <characterkinematic/PxCapsuleController.h>


enum ControllerCollisionFlags : uint8_t
{
	CONTROLLER_COLLISION_SIDES = 1 << 0,
	CONTROLLER_COLLISION_UP = 1 << 1,
	CONTROLLER_COLLISION_DOWN = 1 << 2,
};

struct CharacterController
{
	physx::PxCapsuleController* controller;
};


void InitCharacterController(CharacterController* controller, float radius, float height, float stepOffset, const vec3& position);
void DestroyCharacterController(CharacterController* controller);

ControllerCollisionFlags MoveCharacterController(CharacterController* controller, const vec3& delta, uint32_t filterMask);
vec3 GetCharacterControllerPosition(CharacterController* controller);
void SetCharacterControllerPosition(CharacterController* controller, const vec3& position);
