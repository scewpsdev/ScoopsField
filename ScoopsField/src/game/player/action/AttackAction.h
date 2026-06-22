#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct Projectile;
struct Entity;
struct Action;
struct Player;
struct Trail;

struct AttackAction
{
	Item* weapon;
	Attack* attack;
	int attackIdx;

	uint32_t button;
	uint32_t cancelButton;

	List<Entity*, 16> hitEntities;

	bool cancelled;
	Projectile* projectile;

	Trail* trail;

	float lastHitTime;
};


void InitAttackAction(Action* action, Item* weapon, bool right, Attack* attack, int attackIdx, uint32_t button, uint32_t cancelButton);
void StartAttackAction(Action* action, Player* player);
void StopAttackAction(Action* action, Player* player);
void UpdateAttackAction(Action* action, Player* player);
