#include "Action.h"

#include "game/Game.h"


extern GameState* game;
extern float gameTime;
extern float deltaTime;


void InitAction(Action* action, ActionType type)
{
	*action = {};
	action->type = type;
	//action->speed = 1.0f;
	action->animationSpeed = 1.0f;
	action->moveSpeed = 1.0f;
}

void AddActionSound(Action* action, Sound* sounds, int numSounds, float time, float volume, float pan)
{
	SDL_assert(action->numSounds < MAX_ACTION_SOUNDS);
	ActionSound* actionSound = &action->sounds[action->numSounds++];
	actionSound->sounds = sounds;
	actionSound->numSounds = numSounds;
	actionSound->time = time;
	actionSound->volume = volume;
	actionSound->pan = pan;
	actionSound->played = false;
}

void UpdateAction(Action* action, struct Player* player, float deltaTime)
{
	action->elapsedTime += deltaTime * action->animationSpeed;

	for (int i = 0; i < action->numSounds; i++)
	{
		ActionSound* sound = &action->sounds[i];
		if (!sound->played && action->elapsedTime >= sound->time)
		{
			int soundIdx = game->random.next() % sound->numSounds;
			PlaySound(&sound->sounds[soundIdx], sound->pan, sound->volume);
			sound->played = true;
		}
	}

	RunActionFunc(Update);
}


void InitActionManager(ActionManager& actions, Model* moveset)
{
	actions.moveset = moveset;
	InitQueue(actions.actions);
}

static void StartActionInternal(ActionManager& actions, Action* action, Player* player)
{
	action->startTime = gameTime;

	InitAnimation(&action->anim, action->animName, action->animMoveset ? action->animMoveset : actions.moveset, action->animationSpeed);
	if (!action->duration)
		action->duration = action->anim.animation->duration;
	//action->speed = action->speed;

	StartAction(action, player);
}

static void StopActionInternal(ActionManager& actions, Action* action, Player* player)
{
	StopAction(action, player);
}

void QueueAction(ActionManager& actions, const Action& action, Player& player)
{
	if (actions.actions.size < actions.actions.capacity)
	{
		QueuePush(actions.actions, action);
		if (actions.actions.size == 1)
			StartActionInternal(actions, QueuePeek(actions.actions), &player);
		else
		{
			int a = 5;
		}
	}
}

void CancelAction(ActionManager& actions, Player& player)
{
	if (Action* currentAction = QueuePeek(actions.actions))
	{
		StopActionInternal(actions, currentAction, &player);
		QueuePop(actions.actions);
		currentAction = QueuePeek(actions.actions);
	}
}

void UpdateActionManager(ActionManager& actions, Player& player)
{
	if (actions.actions.size > 0)
	{
		Action* currentAction = QueuePeek(actions.actions);
		if (currentAction->startTime > 0)
		{
			bool shouldFinish = currentAction->elapsedTime >= currentAction->duration ||
				currentAction->followUpCancelTime && currentAction->elapsedTime >= currentAction->followUpCancelTime && actions.actions.size > 1 /*&& currentAction->type == QueuePeekAt(actions.actions, 1)->type*/;
			if (shouldFinish)
			{
				StopActionInternal(actions, currentAction, &player);
				QueuePop(actions.actions);
				currentAction = QueuePeek(actions.actions);
			}
		}

		if (currentAction)
		{
			if (!currentAction->startTime)
				StartActionInternal(actions, currentAction, &player);
			UpdateAction(currentAction, &player, deltaTime);
		}
	}
}
