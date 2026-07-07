#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitParryAction(Action* action, Item* weapon, int side)
{
	InitAction(action, ACTION_TYPE_PARRY);

	if (side == 0)
	{
		action->rightAnimName = "parry";
		action->rightAnimMoveset = &weapon->moveset;
		action->rightAnimBlendDuration = 0.0f;

		if (weapon->twoHanded)
		{
			action->leftAnimName = "parry";
			action->leftAnimMoveset = &weapon->moveset;
			action->leftAnimBlendDuration = 0.0f;
		}
	}
	else
	{
		action->leftAnimName = "parry";
		action->leftAnimMoveset = &weapon->moveset;
		action->leftAnimBlendDuration = 0.0f;
		action->leftAnimMirror = true;

		if (weapon->twoHanded)
		{
			action->rightAnimName = "parry";
			action->rightAnimMoveset = &weapon->moveset;
			action->rightAnimBlendDuration = 0.0f;
			action->rightAnimMirror = true;
		}
	}

	action->parry.weapon = weapon;
}

void StartParryAction(Action* action, Player* player)
{
	player->blockItem = action->parry.weapon;
}

void StopParryAction(Action* action, Player* player)
{
	player->blockItem = nullptr;
}

void UpdateParryAction(Action* action, Player* player)
{
}
