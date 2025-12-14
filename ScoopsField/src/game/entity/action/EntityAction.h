#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "utils/Queue.h"

#include "game/entity/Entity.h"

#include "EntityAttackAction.h"
#include "EntityStaggerAction.h"
#include "EntityDeathAction.h"


enum EntityActionType
{
	ENTITY_ACTION_TYPE_NONE,

	ENTITY_ACTION_TYPE_ATTACK,
	ENTITY_ACTION_TYPE_STAGGER,
	ENTITY_ACTION_TYPE_DEATH,

	ENTITY_ACTION_TYPE_LAST
};

struct EntityAction
{
	EntityActionType type;
	const char* animName;
	Model* animMoveset;
	bool fullBody;

	float duration;
	float speed;
	float walkSpeed;
	float followUpCancelTime;

	float startTime;
	float elapsedTime;
	AnimationPlayback anim;

	union
	{
		EntityAttackAction attack;
		EntityStaggerAction stagger;
		EntityDeathAction death;
	};
};

struct EntityActionManager
{
	Model* moveset;
	Queue<EntityAction, 2> actions;
};


#define EntityActionCase(func, type, TYPE) case ENTITY_ACTION_TYPE_ ## TYPE: func ## Entity ## type ## Action(action, entity); break;

#define RunEntityActionFunc(func) \
switch(action->type) { \
EntityActionCase(func, Attack, ATTACK) \
EntityActionCase(func, Stagger, STAGGER) \
EntityActionCase(func, Death, DEATH) \
default: SDL_assert(false); break; \
}

inline void StartAction(EntityAction* action, Entity* entity)
{
	RunEntityActionFunc(Start);
}

inline void StopAction(EntityAction* action, Entity* entity)
{
	RunEntityActionFunc(Stop);
}

inline void UpdateAction(EntityAction* action, Entity* entity, float deltaTime)
{
	action->elapsedTime += deltaTime;
	RunEntityActionFunc(Update);
}


void InitAction(EntityAction* action, EntityActionType type);

void InitActionManager(EntityActionManager& actions, Model* moveset);
void UpdateActionManager(EntityActionManager& actions, Entity& entity);

void QueueAction(EntityActionManager& actions, const EntityAction& action, Entity& entity);
void CancelAction(EntityActionManager& actions, Entity& entity);
