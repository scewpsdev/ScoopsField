#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitEquipAction(Action* action, Item* rightWeapon, Item* leftWeapon, int dstLoadout)
{
	InitAction(action, ACTION_TYPE_EQUIP);

	if (rightWeapon)
	{
		action->rightAnimName = "equip";
		action->rightAnimMoveset = &rightWeapon->moveset;
		action->rightAnimBlendDuration = 0.0f;

		if (rightWeapon->twoHanded)
		{
			action->leftAnimName = "equip";
			action->leftAnimMoveset = &rightWeapon->moveset;
			action->leftAnimBlendDuration = 0.0f;

			SDL_assert(!leftWeapon);
		}

		action->animationSpeed = 1.0f;
		//action->moveSpeed = 0.5f;

		if (rightWeapon->equipSound)
			AddActionSound(action, rightWeapon->equipSound, 0, 1, 1, 0);
	}

	if (leftWeapon)
	{
		action->leftAnimName = "equip";
		action->leftAnimMoveset = &leftWeapon->moveset;
		action->leftAnimBlendDuration = 0.0f;
		action->leftAnimMirror = true;

		if (leftWeapon->twoHanded)
		{
			action->rightAnimName = "equip";
			action->rightAnimMoveset = &leftWeapon->moveset;
			action->rightAnimBlendDuration = 0.0f;
			action->rightAnimMirror = true;

			SDL_assert(!rightWeapon);
		}

		action->animationSpeed = 1.0f;
		//action->moveSpeed = 0.5f;

		if (leftWeapon->equipSound)
			AddActionSound(action, leftWeapon->equipSound, 0, 1, 1, 0);
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
