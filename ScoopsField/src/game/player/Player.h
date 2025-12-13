#pragma once

#include "game/entity/Entity.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/CharacterController.h"

#include "action/Action.h"

#include "game/item/Item.h"


enum CameraMode
{
	CAMERA_MODE_FIRST_PERSON,
	CAMERA_MODE_FREE,
};

struct Player : Entity
{
	vec3 position;
	float rotation;
	float verticalVelocity;
	bool grounded;
	bool moving;

	float walkSpeed;

	CameraMode cameraMode;

	Model model;
	AnimationState anim;

	Node* rightWeaponNode;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;

	Item* rightWeapon;

	CharacterController controller;

	ActionManager actions;
};

mat4 GetRightWeaponTransform(Player* player);
