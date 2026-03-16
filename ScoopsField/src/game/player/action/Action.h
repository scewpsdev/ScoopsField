#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "audio/Audio.h"

#include "utils/Queue.h"

#include "AttackAction.h"
#include "EquipAction.h"
#include "PickUpAction.h"
#include "DropAction.h"
#include "SitAction.h"


enum ActionType
{
	ACTION_TYPE_NONE,

	ACTION_TYPE_ATTACK,
	ACTION_TYPE_EQUIP,
	ACTION_TYPE_PICKUP,
	ACTION_TYPE_DROP,
	ACTION_TYPE_SIT,

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

	const char* rightAnimName;
	Model* rightAnimMoveset;
	float rightAnimBlendDuration;

	const char* leftAnimName;
	Model* leftAnimMoveset;
	float leftAnimBlendDuration;

	const char* bodyAnimName;
	Model* bodyAnimMoveset;
	float bodyAnimBlendDuration;

	AnimationPlayback rightAnim;
	AnimationPlayback leftAnim;
	AnimationPlayback bodyAnim;

	float animationSpeed;
	bool rootMotion;
	bool fullBodyAnim;
	bool lockPlayerRotation;

	float duration;
	//float speed;
	float moveSpeed;
	float followUpCancelTime;

	float startTime;
	float elapsedTime;

	bool overrideRightWeapon;
	Item* rightWeapon;
	bool overrideLeftWeapon;
	Item* leftWeapon;

#define MAX_ACTION_SOUNDS 8
	ActionSound sounds[MAX_ACTION_SOUNDS];
	int numSounds;

	union
	{
		AttackAction attack;
		EquipAction equip;
		PickUpAction pickup;
		DropAction drop;
		SitAction sit;
	};
};

struct ActionManager
{
	Model* moveset;
	Model* bodyMoveset;
	Queue<Action, 2> actions;
};


#define ActionCase(func, type, TYPE) case ACTION_TYPE_ ## TYPE: func ## type ## Action(action, player); break;

#define RunActionFunc(func) \
switch(action->type) { \
ActionCase(func, Attack, ATTACK) \
ActionCase(func, Equip, EQUIP) \
ActionCase(func, PickUp, PICKUP) \
ActionCase(func, Drop, DROP) \
ActionCase(func, Sit, SIT) \
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
void AddActionSound(Action* action, Sound* sounds, int numSounds = 1, float time = 0, float volume = 1, float pan = 0);

void InitActionManager(ActionManager& actions, Model* moveset, Model* bodyMoveset);
void UpdateActionManager(ActionManager& actions, struct Player& player);

void QueueAction(ActionManager& actions, const Action& action, struct Player& player);
void CancelAction(ActionManager& actions, struct Player& player);
