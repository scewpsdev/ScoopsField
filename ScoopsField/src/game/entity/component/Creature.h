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

struct ColliderData
{
	char bone[64];
	ColliderType type;
	bool trigger;
	vec3 size;
	vec3 offset;
	vec3 rotation;
};

struct Creature : EntityBase
{
	float lookDirection;

	bool moving;

	Model* model;
	AnimationState anim;
	RigidBody body;

#define MAX_CREATURE_HITBOXES 32
	//HashMap<uint32_t, ColliderData, MAX_CREATURE_HITBOXES> hitboxData;
	HashMap<uint32_t, RigidBody, MAX_CREATURE_HITBOXES> hitboxes;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;
	AnimationPlayback attackAnim;

	EntityActionManager actions;

	int health;
	int maxHealth;

	vec3 targetPosition;
	int currentPath[MAX_NAVMESH_PATH_LENGTH];
	int currentPathLength;

	Sound* hitSound;
};


void InitCreature(Creature* creature, const char* model, float lookDirection, int health);
void DestroyCreature(Creature* creature);

bool HitCreature(Creature* creature, HitParams* hit, Entity* by);

void UpdateCreature(Creature* creature);
void RenderCreature(Creature* creature);
