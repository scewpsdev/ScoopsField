#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "audio/Audio.h"

#include "utils/Queue.h"

#include "AttackAction.h"


enum ActionType
{
	ACTION_TYPE_NONE,

	ACTION_TYPE_ATTACK,

	ACTION_TYPE_LAST
};

struct ActionSound
{
	Sound* sounds;
	int numSounds;
	float time;
	float volume;
	float pan;

	bool played;
};

struct Action
{
	ActionType type;
	const char* animName;
	Model* animMoveset;
	float animationSpeed;

	float duration;
	float speed;
	float moveSpeed;
	float followUpCancelTime;

	float startTime;
	float elapsedTime;
	AnimationPlayback anim;

#define MAX_ACTION_SOUNDS 8
	ActionSound sounds[MAX_ACTION_SOUNDS];
	int numSounds;

	union
	{
		AttackAction attack;
	};
};

struct ActionManager
{
	Model* moveset;
	Queue<Action, 2> actions;
};


#define ActionCase(func, type, TYPE) case ACTION_TYPE_ ## TYPE: func ## type ## Action(action, player); break;

#define RunActionFunc(func) \
switch(action->type) { \
ActionCase(func, Attack, ATTACK) \
default: SDL_assert(false); break; \
}

inline void StartAction(Action* action, struct Player* player)
{
	RunActionFunc(Start);
}

inline void StopAction(Action* action, struct Player* player)
{
	RunActionFunc(Stop);
}

void UpdateAction(Action* action, struct Player* player, float deltaTime);

void InitAction(Action* action, ActionType type);
void AddActionSound(Action* action, Sound* sounds, int numSounds, float time, float volume, float pan);

void InitActionManager(ActionManager& actions, Model* moveset);
void UpdateActionManager(ActionManager& actions, struct Player& player);

void QueueAction(ActionManager& actions, const Action& action, struct Player& player);
