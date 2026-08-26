#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct PickUpAction
{
	Item* item;
};


void InitPickUpAction(struct Action* action, Item* item);
void StartPickUpAction(struct Action* action, struct Player* player);
void StopPickUpAction(struct Action* action, struct Player* player);
void UpdatePickUpAction(struct Action* action, struct Player* player);
