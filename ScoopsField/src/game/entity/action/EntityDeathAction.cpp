#include "EntityDeathAction.h"

#include "EntityAction.h"


void InitEntityDeathAction(EntityAction* action)
{
	InitAction(action, ENTITY_ACTION_TYPE_DEATH);
	action->animName = "death";
	action->duration = 1000;
}

void StartEntityDeathAction(EntityAction* action, Entity* entity)
{
}

void StopEntityDeathAction(EntityAction* action, Entity* entity)
{
}

void UpdateEntityDeathAction(EntityAction* action, Entity* entity)
{
}
