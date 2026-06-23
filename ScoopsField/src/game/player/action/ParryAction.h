#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct ParryAction
{
	Item* weapon;
};

struct Action;
struct Player;


void InitParryAction(Action* action, Item* weapon, int side);
void StartParryAction(Action* action, Player* player);
void StopParryAction(Action* action, Player* player);
void UpdateParryAction(Action* action, Player* player);
