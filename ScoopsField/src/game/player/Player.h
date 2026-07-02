#pragma once

#include "game/entity/EntityBase.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/CharacterController.h"

#include "action/Action.h"

#include "game/item/Item.h"

#include "utils/Simplex.h"


#define BLOCK_STAGGER_DURATION 0.5f
#define GUARD_BREAK_STAGGER_DURATION 1.5f


enum CameraMode
{
	CAMERA_MODE_FIRST_PERSON,
	CAMERA_MODE_FREE,
};

struct HitParams;

struct Player : EntityBase
{
	//vec3 position;
	float rotation;
	float pitch, yaw;
	float cameraHeight;

	float upperBodyTurn;
	bool resetUpperBodyTurn;

	vec3 velocity;
	bool grounded;
	bool moving;
	bool sprinting;
	bool ducked;
	float duckTimer;

	float walkSpeed;

	CameraMode cameraMode;

	Model* model;
	AnimationState anim;

	Model* bodyModel;
	AnimationState bodyAnim;

	AnimationState rightWeaponAnim;
	AnimationPlayback rightWeaponAnimation;

	AnimationState leftWeaponAnim;
	AnimationPlayback leftWeaponAnimation;

	Animation* lastRightAnim;
	float lastRightAnimTimer;
	bool lastRightAnimLoop;
	bool lastRightAnimMirror;

	Animation* rightBlendAnim;
	float rightBlendAnimTimer;
	bool rightBlendAnimLoop;
	bool rightBlendAnimMirror;
	float rightBlendDuration;
	float rightBlendStart;

	Animation* lastLeftAnim;
	float lastLeftAnimTimer;
	bool lastLeftAnimLoop;
	bool lastLeftAnimMirror;

	Animation* leftBlendAnim;
	float leftBlendAnimTimer;
	bool leftBlendAnimLoop;
	bool leftBlendAnimMirror;
	float leftBlendDuration;
	float leftBlendStart;

	Animation* lastBodyAnim;
	float lastBodyAnimTimer;
	bool lastBodyAnimLoop;

	Animation* bodyBlendAnim;
	float bodyBlendAnimTimer;
	bool bodyBlendAnimLoop;
	float bodyBlendDuration;
	float bodyBlendStart;

	Node* rightWeaponNode, * leftWeaponNode;
	Node* rightShoulderNode, * leftShoulderNode;
	Node* neckNode, * spineNode, * spine1Node, * spine2Node, * pelvisNode;

	Node* rootNode;
	vec3 rootMotion;
	float rootMotionAngle;
	mat4 lastRootNodeTransform;
	float lastRootMotionUpdate;
	Animation* lastActionAnimation;
	float lastActionAnimationTimer;
	float currentRootMotionRotation;

	AnimationPlayback idleAnim;
	AnimationPlayback bodyIdleAnim, bodyRunAnim, bodyStrafeAnim, bodyDuckAnim, bodySneakAnim, bodySneakStrafeAnim, bodyFallAnim, bodyFallDuckAnim;

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
	float lastGroundedTime;
	float lastLandedTime;
	float lastProjectileHit;
	bool lastProjectileHitHeadshot;
	float lastBlockTime;
	int lastBlockSide;
	bool lastBlockParry;
	bool lastBlockStagger;

	RigidBody kinematicBody;

	ActionManager actions;

	Entity* interactTarget;

	Item* blockItem;
	bool parry;

	int health;
	int maxHealth;

	float stamina;
	bool exhausted;

	float lastHit;
};


Action* GetCurrentAction(Player* player);

Item* GetRightWeapon(Player* player);
Item* GetLeftWeapon(Player* player);

mat4 GetRightWeaponTransform(Player* player);
mat4 GetLeftWeaponTransform(Player* player);

void MovePlayer(Player* player, vec3 delta);
void TeleportPlayer(Player* player, vec3 position);

bool HitPlayer(Player* player, HitParams* hit, Entity* by);
bool GiveItem(Player* player, Item* item);
bool DropItem(Player* player, Item* item);

void OnControllerHit(Player* player, vec3 position, vec3 normal, float length, vec3 direction);
