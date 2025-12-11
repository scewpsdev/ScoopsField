#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/CharacterController.h"


enum CameraMode
{
	CAMERA_MODE_FIRST_PERSON,
	CAMERA_MODE_FREE,
};

struct AnimationPlayback
{
	char name[32] = "";
	float speed = 1.0f;
	bool loop = true;
	bool mirror = false;

	Animation* animation;
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
	float animationTimer;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;
	AnimationPlayback* currentAnim;

	CharacterController controller;
};
