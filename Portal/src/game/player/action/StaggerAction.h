#pragma once

#include <stdint.h>


struct Item;
struct Action;
struct Player;

struct StaggerAction
{
	Item* blockWeapon;
};


void InitStaggerAction(Action* action, Item* weapon);
void InitStaggerAction(Action* action, float duration);
void StartStaggerAction(Action* action, Player* player);
void StopStaggerAction(Action* action, Player* player);
void UpdateStaggerAction(Action* action, Player* player);
