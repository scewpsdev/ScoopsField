#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct AttackAction
{
	Item* weapon;
	Attack* attack;
	int attackIdx;

	List<RigidBody*, 16> hitEntities;
};


void InitAttackAction(struct Action* action, Item* weapon, Attack* attack, int attackIdx);
void StartAttackAction(struct Action* action, struct Player* player);
void StopAttackAction(struct Action* action, struct Player* player);
void UpdateAttackAction(struct Action* action, struct Player* player);
