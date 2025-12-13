#include "AttackAction.h"

#include "Action.h"


void InitAttackAction(Action* action, Item* weapon)
{
	InitAction(action, ACTION_TYPE_ATTACK);
	action->animName = "attack1";
	action->animMoveset = &weapon->moveset;
}

void UpdateAttackAction(Action* action)
{
	// do dmg raycast
}
