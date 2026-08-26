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
	action->idleAnimStrength = 1.0f;

	action->rightAnimBlendDuration = 0.2f;
	action->leftAnimBlendDuration = 0.2f;
	action->bodyAnimBlendDuration = 0.2f;
}

void AddActionSound(Action* action, Sound* sound, float time, float volume, float speed, float pan)
{
	SDL_assert(action->numEvents < MAX_ACTION_EVENTS);
	ActionEvent* actionEvent = &action->events[action->numEvents++];
	*actionEvent = { };
	actionEvent->time = time;
	actionEvent->sound = sound;
	actionEvent->volume = volume;
	actionEvent->speed = speed;
	actionEvent->pan = pan;
	actionEvent->triggered = false;
}

void AddActionEffect(Action* action, const char* effect, float time, vec3 localPosition)
{
	SDL_assert(action->numEvents < MAX_ACTION_EVENTS);
	ActionEvent* actionEvent = &action->events[action->numEvents++];
	*actionEvent = { };
	actionEvent->time = time;
	actionEvent->effectPath = effect;
	actionEvent->effectPosition = localPosition;
	actionEvent->triggered = false;
}

void UpdateAction(Action* action, struct Player* player, float deltaTime)
{
	action->elapsedTime += deltaTime * action->animationSpeed;

	for (int i = 0; i < action->numEvents; i++)
	{
		ActionEvent* event = &action->events[i];
		if (!event->triggered && action->elapsedTime >= event->time)
		{
			if (event->sound)
			{
				uint32_t handle = PlaySound(event->sound, event->pan, event->volume);
				SetSoundRelativeSpeed(handle, event->speed);
			}
			if (event->effectPath)
			{
				ParticleEffect* effect = (ParticleEffect*)CreateEntity();
				mat4 transform = GetRightWeaponTransform(player) * mat4::Translate(event->effectPosition);
				LoadParticleEffect(effect, event->effectPath, transform.translation(), transform.rotation());
				effect->destroyOnFinish = true;
				event->effect = effect;
			}
			event->triggered = true;
		}
		if (event->triggered && event->effect)
		{
			event->effect->position = GetRightWeaponTransform(player) * event->effectPosition;
		}
	}

	RunActionFunc(Update);
}


void InitActionManager(ActionManager& actions, Model* moveset, Model* bodyMoveset)
{
	actions.moveset = moveset;
	actions.bodyMoveset = bodyMoveset;
	InitQueue(actions.actions);
}

static void StartActionInternal(ActionManager& actions, Action* action, Player* player)
{
	action->startTime = gameTime;

	if (action->rightAnimName)
		InitAnimation(&action->rightAnim, action->rightAnimName, action->rightAnimMoveset ? action->rightAnimMoveset : actions.moveset, action->animationSpeed, false, false);
	if (action->leftAnimName)
		InitAnimation(&action->leftAnim, action->leftAnimName, action->leftAnimMoveset ? action->leftAnimMoveset : actions.moveset, action->animationSpeed, false, false);
	if (action->bodyAnimName)
		InitAnimation(&action->bodyAnim, action->bodyAnimName, action->bodyAnimMoveset ? action->bodyAnimMoveset : actions.bodyMoveset, action->animationSpeed, false, false);
	if (action->rightItemAnimName)
		InitAnimation(&action->rightWeaponAnim, action->rightItemAnimName, action->rightItemAnimMoveset, 1.0f, false, false);
	if (action->leftItemAnimName)
		InitAnimation(&action->leftWeaponAnim, action->leftItemAnimName, action->leftItemAnimMoveset, 1.0f, false, false);

	if (!action->duration)
	{
		action->duration = max(max(action->rightAnimName && action->rightAnim.animation ? action->rightAnim.animation->duration : 0, action->leftAnimName && action->leftAnim.animation ? action->leftAnim.animation->duration : 0), action->bodyAnimName && action->bodyAnim.animation ? action->bodyAnim.animation->duration : 0);
	}
	//action->speed = action->speed;

	StartAction(action, player);
}

static void StopActionInternal(ActionManager& actions, Action* action, Player* player)
{
	StopAction(action, player);
}

void QueueAction(ActionManager& actions, const Action& action, Player& player)
{
	SDL_assert(!action.startTime);
	if (actions.actions.size < actions.actions.capacity)
	{
		QueuePush(actions.actions, action);
		if (actions.actions.size == 1)
			StartActionInternal(actions, QueuePeek(actions.actions), &player);
	}
}

void CancelAction(ActionManager& actions, Player& player)
{
	if (Action* currentAction = QueuePeek(actions.actions))
	{
		StopActionInternal(actions, currentAction, &player);
		QueuePop(actions.actions);
		currentAction = QueuePeek(actions.actions);
		if (currentAction && !currentAction->startTime)
			StartActionInternal(actions, currentAction, &player);
	}
}

void ClearQueuedAction(ActionManager& actions)
{
	if (actions.actions.size == 2)
		QueuePopEnd(actions.actions);
}

void UpdateActionManager(ActionManager& actions, Player& player)
{
	if (actions.actions.size > 0)
	{
		Action* action = QueuePeek(actions.actions);

		if (action->startTime > 0)
		{
			bool shouldFinish = action->elapsedTime >= action->duration ||
				action->followUpCancelTime && action->elapsedTime >= action->followUpCancelTime && actions.actions.size > 1 /*&& action->type == QueuePeekAt(actions.actions, 1)->type*/;
			if (shouldFinish)
			{
				StopActionInternal(actions, action, &player);
				QueuePop(actions.actions);
				action = QueuePeek(actions.actions);
			}
		}

		if (action)
		{
			if (!action->startTime)
				StartActionInternal(actions, action, &player);
			UpdateAction(action, &player, deltaTime);
		}
	}
}
