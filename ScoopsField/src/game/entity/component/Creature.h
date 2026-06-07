#pragma once

#include "game/Navmesh.h"

#include "game/entity/EntityBase.h"	

#include "game/entity/action/EntityAction.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/RigidBody.h"


struct HitParams;

enum ColliderType
{
	COLLIDER_TYPE_NULL = 0,

	COLLIDER_TYPE_BOX,
	COLLIDER_TYPE_CAPSULE,
	COLLIDER_TYPE_SPHERE,

	COLLIDER_TYPE_LAST
};

struct EntityAttack
{
	const char* name;
	const char* animation;
	float animationSpeed;
	bool firstAttack;

	//bool stance;
	//const char* stanceFollowUp;
	//bool projectileShoot;
	//float projectileShootTime;

	vec2 rangeTriggerWindow;
	vec2 angleTriggerWindow;
	vec2 damageWindow;
	float followUpCancelTime;
	float damageMultiplier;

	const char* followUp;
	float followUpChance;

#define MAX_ATTACK_SOUNDS 8
	AttackSound sounds[MAX_ATTACK_SOUNDS];
	int numSounds;

#define MAX_ATTACK_EFFECTS 8
	AttackEffect effects[MAX_ATTACK_EFFECTS];
	int numEffects;
};

struct Creature : EntityBase
{
	float lookDirection;

	bool moving;
	vec3 fsu;
	float targetDirection;

	Model* model;
	AnimationState anim;
	RigidBody body;

#define MAX_CREATURE_HITBOXES 32
	//HashMap<uint32_t, ColliderData, MAX_CREATURE_HITBOXES> hitboxData;
	HashMap<uint32_t, RigidBody, MAX_CREATURE_HITBOXES> hitboxes;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;
	AnimationPlayback attackAnim;

	Animation* blendAnim;
	float blendAnimTimer;
	bool blendAnimLoop;
	float blendDuration;
	float blendStart;

	Animation* lastAnim;
	float lastAnimTimer;
	bool lastAnimLoop;

	Node* rightWeaponNode;

	Node* rootNode;
	vec3 rootMotionVelocity;
	float rootMotionAngle;
	mat4 lastRootNodeTransform;
	float lastRootMotionUpdate;
	Animation* lastActionAnimation;
	float lastActionAnimationTimer;
	float currentRootMotionRotation;

#define MAX_ENTITY_ATTACKS 16
	EntityAttack attacks[MAX_ENTITY_ATTACKS];
	int numAttacks;

	EntityActionManager actions;

	int health;
	int maxHealth;
	float walkSpeed;
	float turnSpeed;

	int damage;
	float weaponRange;

	vec3 eyePosition;
	float detectionRange;
	float detectionAngle;
	Entity* target;
	vec3 targetPosition;
	Entity* searchEntity;
	int currentPath[MAX_NAVMESH_PATH_LENGTH];
	int currentPathLength;

	Sound* hitSound;
};


void InitCreature(Creature* creature, const char* model, float lookDirection, int health);
void DestroyCreature(Creature* creature);

bool HitCreature(Creature* creature, HitParams* hit, Entity* by);

mat4 GetRightWeaponTransform(Creature* creature);

void UpdateCreature(Creature* creature);
void RenderCreature(Creature* creature);
