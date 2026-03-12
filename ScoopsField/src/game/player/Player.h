#pragma once

#include "game/entity/Entity.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/CharacterController.h"

#include "action/Action.h"

#include "game/item/Item.h"

#include "utils/Simplex.h"


enum CameraMode
{
	CAMERA_MODE_FIRST_PERSON,
	CAMERA_MODE_FREE,
};

struct Player : Entity
{
	vec3 position;
	float rotation;
	vec3 velocity;
	bool grounded;
	bool moving;
	bool sprinting;

	float walkSpeed;

	CameraMode cameraMode;

	Model model;
	AnimationState anim;

	Animation* lastRightAnim;
	float lastRightAnimTimer;
	bool lastRightAnimLoop;

	Animation* rightBlendAnim;
	float rightBlendAnimTimer;
	bool rightBlendAnimLoop;
	float rightBlendDuration;
	float rightBlendStart;

	Animation* lastLeftAnim;
	float lastLeftAnimTimer;
	bool lastLeftAnimLoop;

	Animation* leftBlendAnim;
	float leftBlendAnimTimer;
	bool leftBlendAnimLoop;
	float leftBlendDuration;
	float leftBlendStart;

	Node* rightWeaponNode, * leftWeaponNode;
	Node* rightShoulderNode, * leftShoulderNode;

	AnimationPlayback idleAnim;

	float viewBobVerticalSpeedAnim;
	vec3 viewBobLookSwayAnim;
	int lastStepIdx;

	#define NUM_LOADOUTS 3
	Item* rightWeapons[NUM_LOADOUTS];
	Item* leftWeapons[NUM_LOADOUTS];
	int currentLoadout;

	CharacterController controller;
	float distanceWalked;
	float lastJumpInput;
	float lastLandedTime;

	RigidBody kinematicBody;

	ActionManager actions;

	int health;
	int maxHealth;

	float stamina;
	bool exhausted;

	float lastHit;
};

mat4 GetRightWeaponTransform(Player* player);

void HitPlayer(Player* player, HitParams hit, Entity* by);
