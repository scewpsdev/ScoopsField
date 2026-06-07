#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "game/player/action/Action.h"

#include "utils/Queue.h"

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

struct Entity;

struct EntityAction
{
	EntityActionType type;
	const char* animName;
	Model* animMoveset;
	bool rootMotion;
	bool fullBody;

	float duration;
	float speed;
	float walkSpeed;
	float turnSpeed;
	float followUpCancelTime;

	AnimationPlayback anim;

#define MAX_ACTION_EVENTS 8
	ActionEvent events[MAX_ACTION_EVENTS];
	int numEvents;

	float startTime;
	float elapsedTime;

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


void InitAction(EntityAction* action, EntityActionType type);
void AddActionSound(EntityAction* action, Sound* sound, float time = 0, float volume = 1, float speed = 1);
void AddActionEffect(EntityAction* action, const char* effect, float time, vec3 localPosition);

void InitActionManager(EntityActionManager& actions, Model* moveset);
void UpdateActionManager(EntityActionManager& actions, Entity& entity);

void QueueAction(EntityActionManager& actions, const EntityAction& action, Entity& entity);
void CancelAction(EntityActionManager& actions, Entity& entity);
