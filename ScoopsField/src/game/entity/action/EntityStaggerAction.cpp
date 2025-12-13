#include "EntityStaggerAction.h"

#include "EntityAction.h"


void InitEntityStaggerAction(EntityAction* action, float duration)
{
	InitAction(action, ENTITY_ACTION_TYPE_STAGGER);
	action->animName = "stagger_hit";
	action->stagger.duration = duration;
}

void StartEntityStaggerAction(EntityAction* action, Entity* entity)
{
	action->speed = action->anim.animation->duration / action->stagger.duration;
}

void StopEntityStaggerAction(EntityAction* action, Entity* entity)
{

}

void UpdateEntityStaggerAction(EntityAction* action, Entity* entity)
{

}
