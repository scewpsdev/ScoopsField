#pragma once

#include "game/Navmesh.h"

#include "game/entity/action/EntityAction.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/RigidBody.h"


struct Entity;
struct HitParams;

struct Creature
{
	float lookDirection;

	bool moving;

	Model* model;
	AnimationState anim;
	RigidBody body;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;
	AnimationPlayback attackAnim;

	EntityActionManager actions;

	int health;
	int maxHealth;

	vec3 targetPosition;
	int currentPath[MAX_NAVMESH_PATH_LENGTH];
	int currentPathLength;
};


void InitCreature(Creature* creature, Entity* entity, const char* model, float lookDirection, int health);
void DestroyCreature(Creature* creature, Entity* entity);

bool HitCreature(Creature* creature, Entity* entity, HitParams* hit, Entity* by);

void UpdateCreature(Creature* creature, Entity* entity);
void RenderCreature(Creature* creature, Entity* entity);
