#pragma once

#include "model/Model.h"
#include "model/Animation.h"

#include "audio/Audio.h"

#include "utils/Queue.h"

#include "AttackAction.h"
#include "ShootAction.h"
#include "EquipAction.h"
#include "UnequipAction.h"
#include "PickUpAction.h"
#include "DropAction.h"
#include "StaggerAction.h"
#include "ParryAction.h"
#include "SitAction.h"
#include "TurnAction.h"


enum ActionType
{
	ACTION_TYPE_NONE,

	ACTION_TYPE_ATTACK,
	ACTION_TYPE_SHOOT,
	ACTION_TYPE_EQUIP,
	ACTION_TYPE_UNEQUIP,
	ACTION_TYPE_PICKUP,
	ACTION_TYPE_DROP,
	ACTION_TYPE_STAGGER,
	ACTION_TYPE_PARRY,
	ACTION_TYPE_SIT,
	ACTION_TYPE_TURN,

	ACTION_TYPE_LAST
};

struct ParticleEffect;

struct ActionEvent
{
	float time;

	Sound* sound;
	float volume;
	float speed;
	float pan;

	const char* effectPath;
	vec3 effectPosition;
	ParticleEffect* effect;

	bool triggered;
};

struct Action
{
	ActionType type;

	const char* rightAnimName;
	Model* rightAnimMoveset;
	float rightAnimBlendDuration;
	bool rightAnimMirror;

	const char* leftAnimName;
	Model* leftAnimMoveset;
	float leftAnimBlendDuration;
	bool leftAnimMirror;

	const char* bodyAnimName;
	Model* bodyAnimMoveset;
	float bodyAnimBlendDuration;

	const char* rightItemAnimName;
	Model* rightItemAnimMoveset;
	float rightItemAnimBlendDuration;

	const char* leftItemAnimName;
	Model* leftItemAnimMoveset;
	float leftItemAnimBlendDuration;

	AnimationPlayback rightAnim;
	AnimationPlayback leftAnim;
	AnimationPlayback bodyAnim;
	AnimationPlayback rightWeaponAnim;
	AnimationPlayback leftWeaponAnim;

	bool overrideRightWeapon;
	Item* rightWeapon;
	bool overrideLeftWeapon;
	Item* leftWeapon;

	float animationSpeed;
	bool rootMotion;
	bool fullBodyAnim;
	bool lockPlayerRotation;
	float followUpCancelTime;

	float duration;
	//float speed;
	float moveSpeed;
	float idleAnimStrength;

#define MAX_ACTION_EVENTS 8
	ActionEvent events[MAX_ACTION_EVENTS];
	int numEvents;

	float startTime;
	float elapsedTime;

	union
	{
		AttackAction attack;
		ShootAction shoot;
		EquipAction equip;
		UnequipAction unequip;
		PickUpAction pickup;
		DropAction drop;
		StaggerAction stagger;
		ParryAction parry;
		SitAction sit;
		TurnAction turn;
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
ActionCase(func, Shoot, SHOOT) \
ActionCase(func, Equip, EQUIP) \
ActionCase(func, Unequip, UNEQUIP) \
ActionCase(func, PickUp, PICKUP) \
ActionCase(func, Drop, DROP) \
ActionCase(func, Stagger, STAGGER) \
ActionCase(func, Parry, PARRY) \
ActionCase(func, Sit, SIT) \
ActionCase(func, Turn, TURN) \
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
void AddActionSound(Action* action, Sound* sound, float time = 0, float volume = 1, float speed = 1, float pan = 0);
void AddActionEffect(Action* action, const char* effect, float time, vec3 localPosition);

void InitActionManager(ActionManager& actions, Model* moveset, Model* bodyMoveset);
void UpdateActionManager(ActionManager& actions, struct Player& player);

void QueueAction(ActionManager& actions, const Action& action, struct Player& player);
void CancelAction(ActionManager& actions, struct Player& player);
void ClearQueuedAction(ActionManager& actions);
