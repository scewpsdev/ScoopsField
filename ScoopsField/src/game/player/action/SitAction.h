#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct SitAction
{
};


void InitSitAction(struct Action* action);
void StartSitAction(struct Action* action, struct Player* player);
void StopSitAction(struct Action* action, struct Player* player);
void UpdateSitAction(struct Action* action, struct Player* player);
