#pragma once

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

struct Player
{
	vec3 position;
	float rotation;
	float verticalVelocity;
	bool grounded;
	bool moving;

	CameraMode cameraMode;

	Model model;
	AnimationState anim;

	Node* rightWeaponNode;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;

	Item* rightWeapon;
	AnimationPlayback weaponIdleAnim;
	AnimationPlayback weaponAttackAnim;

	CharacterController controller;

	ActionManager actions;
};
