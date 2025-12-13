#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "utils/Queue.h"

#include "AttackAction.h"


struct AnimationPlayback
{
	char name[32] = "";
	float speed = 1.0f;
	bool loop = true;
	bool mirror = false;

	Animation* animation;

	float timer;
};

enum ActionType
{
	ACTION_TYPE_NONE,

	ACTION_TYPE_ATTACK,

	ACTION_TYPE_LAST
};

struct Action
{
	ActionType type;
	const char* animName;
	Model* animMoveset;

	float followUpCancelTime;

	float duration;
	float startTime;
	float elapsedTime;
	AnimationPlayback anim;

	union
	{
		AttackAction attack;
	};
};

struct ActionManager
{
	Queue<Action, 2> actions;
};


inline void UpdateAction(Action* action, float deltaTime)
{
	action->elapsedTime += deltaTime;

	switch (action->type)
	{
	case ACTION_TYPE_ATTACK: UpdateAttackAction(action); break;
	default: SDL_assert(false); break;
	}
}


void InitAnimation(AnimationPlayback* animation, const char* name, Model* moveset, float speed = 1.0f, bool loop = false, bool mirror = false);

void InitAction(Action* action, ActionType type);

void InitActionManager(ActionManager& actions);
void UpdateActionManager(ActionManager& actions);

void QueueAction(ActionManager& actions, const Action& action);
