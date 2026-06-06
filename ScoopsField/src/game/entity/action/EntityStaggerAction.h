#pragma once

#include "physics/RigidBody.h"


struct Entity;
struct EntityAction;

struct EntityStaggerAction
{
	float duration;
};


void InitEntityStaggerAction(EntityAction* action, float duration, const char* animation);
void StartEntityStaggerAction(EntityAction* action, Entity* entity);
void StopEntityStaggerAction(EntityAction* action, Entity* entity);
void UpdateEntityStaggerAction(EntityAction* action, Entity* entity);
