#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitEquipAction(Action* action, Item* weapon, int dstLoadout)
{
	InitAction(action, ACTION_TYPE_EQUIP);

	if (weapon)
	{
		action->rightAnimName = "equip";
		action->rightAnimMoveset = &weapon->moveset;
		action->rightAnimBlendDuration = 0.0f;

		if (weapon->twoHanded)
		{
			action->leftAnimName = "equip";
			action->leftAnimMoveset = &weapon->moveset;
			action->leftAnimBlendDuration = 0.0f;
		}
		action->animationSpeed = 1.0f;
		//action->moveSpeed = 0.5f;
	}
	else
	{
		action->rightAnimName = "equip";
		action->rightAnimMoveset = &game->player.model;
		action->rightAnimBlendDuration = 0.0f;

		action->leftAnimName = "equip";
		action->leftAnimMoveset = &game->player.model;
		action->leftAnimBlendDuration = 0.0f;
	}

	action->equip.dstLoadout = dstLoadout;

	//AddActionSound(action, game->swingSounds, 3, attack->damageWindow.x, 1, (attackIdx % 2 * -2 + 1) * 0.2f);
}

void StartEquipAction(Action* action, Player* player)
{
	if (action->equip.dstLoadout != -1)
	{
		player->currentLoadout = action->equip.dstLoadout;
	}
}

void StopEquipAction(Action* action, Player* player)
{
}

void UpdateEquipAction(Action* action, Player* player)
{
}
