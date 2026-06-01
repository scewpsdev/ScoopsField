#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct Projectile;

struct AttackAction
{
	Item* weapon;
	Attack* attack;
	int attackIdx;

	uint32_t button;

	List<RigidBody*, 16> hitEntities;

	Projectile* projectile;

	float lastHitTime;
};


void InitAttackAction(struct Action* action, Item* weapon, Attack* attack, int attackIdx, uint32_t button);
void StartAttackAction(struct Action* action, struct Player* player);
void StopAttackAction(struct Action* action, struct Player* player);
void UpdateAttackAction(struct Action* action, struct Player* player);
