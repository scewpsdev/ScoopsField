#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct Action;
struct Player;

struct ShootAction
{
};


void InitShootAction(Action* action, Item* weapon);
void StartShootAction(Action* action, Player* player);
void StopShootAction(Action* action, Player* player);
void UpdateShootAction(Action* action, Player* player);
