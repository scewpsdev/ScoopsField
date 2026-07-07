#pragma once

#include "physics/RigidBody.h"

#include "game/item/Item.h"

#include "utils/List.h"


struct TurnAction
{
};

struct Action;
struct Player;


void InitTurnAction(Action* action, int direction);
void StartTurnAction(Action* action, Player* player);
void StopTurnAction(Action* action, Player* player);
void UpdateTurnAction(Action* action, Player* player);
