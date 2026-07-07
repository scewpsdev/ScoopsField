#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct Entity;
struct EntityAction;
struct EntityAttack;

struct EntityAttackAction
{
	EntityAttack* attack;
	int attackIdx;

	List<Entity*, 16> hitEntities;
};


void InitEntityAttackAction(EntityAction* action, EntityAttack* attack, int attackIdx);
void StartEntityAttackAction(EntityAction* action, Entity* entity);
void StopEntityAttackAction(EntityAction* action, Entity* entity);
void UpdateEntityAttackAction(EntityAction* action, Entity* entity);
