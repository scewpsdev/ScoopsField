#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct DropAction
{
	bool dropped;
};


void InitDropAction(struct Action* action);
void StartDropAction(struct Action* action, struct Player* player);
void StopDropAction(struct Action* action, struct Player* player);
void UpdateDropAction(struct Action* action, struct Player* player);
