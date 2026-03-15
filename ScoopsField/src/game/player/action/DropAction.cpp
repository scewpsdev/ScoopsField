#include "DropAction.h"

#include "Action.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitDropAction(Action* action)
{
	InitAction(action, ACTION_TYPE_DROP);

	action->rightAnimName = "drop";
	action->rightAnimMoveset = nullptr;
	action->rightAnimBlendDuration = 0.05f;

	action->overrideRightWeapon = true;
	action->rightWeapon = nullptr;

	action->drop.dropped = false;

	AddActionSound(action, &game->items.clothSound, 1, 0, 1, 0);
}

void StartDropAction(Action* action, Player* player)
{
	if (!GetRightWeapon(player))
		CancelAction(player->actions, *player);
}

void StopDropAction(Action* action, Player* player)
{
}

void UpdateDropAction(Action* action, Player* player)
{
	if (action->elapsedTime > 2 / 24.0f && !action->drop.dropped)
	{
		DropItem(player, GetRightWeapon(player));
		action->drop.dropped = true;
	}
}
