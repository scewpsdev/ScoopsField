#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct Projectile;
struct Entity;
struct Action;
struct Player;
struct Trail;

enum ItemActionType
{
	ITEM_ACTION_NULL = 0,

	ITEM_ACTION_AXE,
	ITEM_ACTION_DIG,
};

struct AttackAction
{
	Item* weapon;
	Attack* attack;
	int attackIdx;
	ItemActionType actionType;

	uint32_t button;
	uint32_t cancelButton;

	bool resetHitbox;

	List<Entity*, 16> hitEntities;
	vec3 lastHitboxTip;

	bool cancelled;
	Projectile* projectile;

	Trail* trail;

	float lastHitTime;
	bool lastHitReflect;

	bool didRaycast;
};


void InitAttackAction(Action* action, Item* weapon, ItemActionType actionType, uint32_t button, uint32_t cancelButton);
void StartAttackAction(Action* action, Player* player);
void StopAttackAction(Action* action, Player* player);
void UpdateAttackAction(Action* action, Player* player);
