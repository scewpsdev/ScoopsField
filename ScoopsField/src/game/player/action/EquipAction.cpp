#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitEquipAction(Action* action, Item* weapon)
{
	InitAction(action, ACTION_TYPE_EQUIP);
	action->animName = "equip";
	action->animMoveset = &weapon->moveset;
	action->animationSpeed = 1.0f;
	action->twoHanded = weapon->twoHanded;
	//action->moveSpeed = 0.5f;

	//AddActionSound(action, game->swingSounds, 3, attack->damageWindow.x, 1, (attackIdx % 2 * -2 + 1) * 0.2f);
}

void StartEquipAction(Action* action, Player* player)
{
}

void StopEquipAction(Action* action, Player* player)
{
}

void UpdateEquipAction(Action* action, Player* player)
{
}
