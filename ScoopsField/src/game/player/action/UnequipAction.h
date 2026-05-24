#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct UnequipAction
{
	Item* weapon;
	int dstLoadout;
};


void InitUnequipAction(struct Action* action, Item* weapon, int dstLoadout = -1);
void StartUnequipAction(struct Action* action, struct Player* player);
void StopUnequipAction(struct Action* action, struct Player* player);
void UpdateUnequipAction(struct Action* action, struct Player* player);
