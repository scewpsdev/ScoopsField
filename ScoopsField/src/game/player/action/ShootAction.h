#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct ShootAction
{
};


void InitShootAction(struct Action* action, Item* weapon);
void StartShootAction(struct Action* action, struct Player* player);
void StopShootAction(struct Action* action, struct Player* player);
void UpdateShootAction(struct Action* action, struct Player* player);
