#include "EntityAttackAction.h"

#include "EntityAction.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/entity/Entity.h"


void InitEntityAttackAction(EntityAction* action, const char* animation, int attackIdx)
{
	InitAction(action, ENTITY_ACTION_TYPE_ATTACK);
	action->animName = animation;
	action->animMoveset = nullptr;
	action->attack.weapon = nullptr;
	action->attack.attack = nullptr;
	action->attack.attackIdx = attackIdx;

	InitList(&action->attack.hitEntities);
}

void StartEntityAttackAction(EntityAction* action, Entity* entity)
{
}

void StopEntityAttackAction(EntityAction* action, Entity* entity)
{
}

void UpdateEntityAttackAction(EntityAction* action, Entity* entity)
{
}
