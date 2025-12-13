#pragma once

#include "physics/RigidBody.h"


struct EntityStaggerAction
{
	float duration;
};


void InitEntityStaggerAction(struct EntityAction* action, float duration);
void StartEntityStaggerAction(struct EntityAction* action, struct Entity* entity);
void StopEntityStaggerAction(struct EntityAction* action, struct Entity* entity);
void UpdateEntityStaggerAction(struct EntityAction* action, struct Entity* entity);
