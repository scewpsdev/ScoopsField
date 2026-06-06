#include "EntityStaggerAction.h"

#include "EntityAction.h"


void InitEntityStaggerAction(EntityAction* action, float duration, const char* animation)
{
	InitAction(action, ENTITY_ACTION_TYPE_STAGGER);
	action->animName = animation;
	action->walkSpeed = 0;
	action->turnSpeed = 0;
	action->fullBody = true;
	action->rootMotion = true;
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
