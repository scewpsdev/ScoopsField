#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct EntityAttackAction
{
	Item* weapon;
	Attack* attack;
	int attackIdx;

	List<RigidBody*, 16> hitEntities;
};


void InitEntityAttackAction(struct EntityAction* action, Item* weapon, Attack* attack, int attackIdx);
void InitEntityAttackAction(struct EntityAction* action, const char* animation, int attackIdx);
void StartEntityAttackAction(struct EntityAction* action, struct Entity* entity);
void StopEntityAttackAction(struct EntityAction* action, struct Entity* entity);
void UpdateEntityAttackAction(struct EntityAction* action, struct Entity* entity);
