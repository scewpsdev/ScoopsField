#include "StaggerAction.h"

#include "Application.h"

#include "Action.h"


void InitStaggerAction(Action* action, Item* weapon)
{
	InitAction(action, ACTION_TYPE_STAGGER);

	action->rightAnimName = "block";
	action->rightAnimMoveset = &weapon->moveset;

	if (weapon->twoHanded)
	{
		action->leftAnimName = "block";
		action->leftAnimMoveset = &weapon->moveset;
	}

	action->moveSpeed = 0.5f;
	action->duration = 1.5f;

	action->stagger.blockWeapon = weapon;
}

void InitStaggerAction(Action* action, float duration)
{
	InitAction(action, ACTION_TYPE_STAGGER);

	action->rightAnimName = "stagger";
	action->rightAnimMoveset = nullptr;
	action->overrideRightWeapon = true;
	action->rightAnimBlendDuration = 0.1f;

	action->leftAnimName = "stagger";
	action->leftAnimMoveset = nullptr;
	action->overrideLeftWeapon = true;
	action->leftAnimBlendDuration = 0.1f;

	action->moveSpeed = 0.5f;

	action->duration = duration;
}

void StartStaggerAction(Action* action, Player* player)
{
}

void StopStaggerAction(Action* action, Player* player)
{
}

void UpdateStaggerAction(Action* action, Player* player)
{
}
