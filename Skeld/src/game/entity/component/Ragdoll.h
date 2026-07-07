#pragma once

#include "game/entity/EntityBase.h"

#include "physics/RigidBody.h"
#include "physics/Articulation.h"


struct Entity;
struct Creature;

struct Ragdoll : EntityBase
{
	Articulation articulation;

#define MAX_RAGDOLL_BONES 32
	HashMap<uint32_t, RigidBody, MAX_RAGDOLL_BONES> bones;

	AnimationState anim;
};


void InitRagdoll(Ragdoll* ragdoll, Creature* creature);
void DestroyRagdoll(Ragdoll* item);

RigidBody* GetRagdollBodyWithName(Ragdoll* ragdoll, const char* name);

void UpdateRagdoll(Ragdoll* ragdoll);
void RenderRagdoll(Ragdoll* ragdoll);
