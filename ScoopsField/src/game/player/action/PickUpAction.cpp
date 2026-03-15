#include "PickUpAction.h"

#include "Action.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitPickUpAction(Action* action, Item* item)
{
	InitAction(action, ACTION_TYPE_PICKUP);

	action->rightAnimName = "pick_up";
	action->rightAnimMoveset = nullptr;

	action->overrideRightWeapon = true;
	action->rightWeapon = nullptr;

	action->pickup.item = item;

	AddActionSound(action, &game->items.clothSound, 1, 0, 1, 0);
}

void StartPickUpAction(Action* action, Player* player)
{
}

void StopPickUpAction(Action* action, Player* player)
{
}

void UpdatePickUpAction(Action* action, Player* player)
{
}
