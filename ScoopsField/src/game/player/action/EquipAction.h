#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct EquipAction
{
	Item* weapon;
	int dstLoadout;
};


void InitEquipAction(struct Action* action, Item* weapon, int dstLoadout = -1);
void StartEquipAction(struct Action* action, struct Player* player);
void StopEquipAction(struct Action* action, struct Player* player);
void UpdateEquipAction(struct Action* action, struct Player* player);
