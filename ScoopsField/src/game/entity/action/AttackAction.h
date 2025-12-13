#pragma once

#include "game/item/Item.h"


struct AttackAction
{
	Item* weapon;
};


void InitAttackAction(struct Action* action, Item* weapon);
void UpdateAttackAction(struct Action* action);
