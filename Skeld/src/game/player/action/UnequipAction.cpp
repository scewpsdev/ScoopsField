#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitUnequipAction(Action* action, Item* weapon, int dstLoadout)
{
	InitAction(action, ACTION_TYPE_UNEQUIP);

	SDL_assert(weapon);

	action->rightAnimName = "unequip";
	action->rightAnimMoveset = &weapon->moveset;
	action->rightAnimBlendDuration = 0.0f;
	action->overrideRightWeapon = true;
	action->rightWeapon = weapon;

	if (weapon->twoHanded)
	{
		action->leftAnimName = "unequip";
		action->leftAnimMoveset = &weapon->moveset;
		action->leftAnimBlendDuration = 0.0f;
		//action->overrideLeftWeapon = true;
		//action->leftWeapon = weapon;
	}

	action->animationSpeed = 1.0f;
	//action->moveSpeed = 0.5f;

	//if (weapon->equipSound)
	//	AddActionSound(action, weapon->equipSound, 1, 0, 1, 0);

	action->unequip.dstLoadout = dstLoadout;

	//AddActionSound(action, game->swingSounds, 3, attack->damageWindow.x, 1, (attackIdx % 2 * -2 + 1) * 0.2f);
}

void StartUnequipAction(Action* action, Player* player)
{
}

void StopUnequipAction(Action* action, Player* player)
{
	if (action->unequip.dstLoadout != -1)
	{
		player->currentLoadout = action->unequip.dstLoadout;
	}
}

void UpdateUnequipAction(Action* action, Player* player)
{
}
