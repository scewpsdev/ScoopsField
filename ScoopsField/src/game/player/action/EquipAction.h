#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct EquipAction
{
	Item* weapon;
	int dstLoadout;
};

struct Action;
struct Player;


void InitEquipAction(Action* action, Item* rightWeapon, Item* leftWeapon, int dstLoadout = -1);
void StartEquipAction(Action* action, Player* player);
void StopEquipAction(Action* action, Player* player);
void UpdateEquipAction(Action* action, Player* player);
