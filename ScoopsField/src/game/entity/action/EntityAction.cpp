#include "EntityAction.h"

#include "Application.h"

#include "game/entity/Entity.h"


extern float gameTime;
extern float deltaTime;


void InitAction(EntityAction* action, EntityActionType type)
{
	*action = {};
	action->type = type;
	action->speed = 1.0f;
	action->walkSpeed = 1.0f;
	action->turnSpeed = 1.0f;
}

void AddActionSound(EntityAction* action, Sound* sound, float time, float volume, float speed)
{
	SDL_assert(action->numEvents < MAX_ACTION_EVENTS);
	ActionEvent* actionEvent = &action->events[action->numEvents++];
	*actionEvent = { };
	actionEvent->time = time;
	actionEvent->sound = sound;
	actionEvent->volume = volume;
	actionEvent->speed = speed;
	actionEvent->triggered = false;
}

void AddActionEffect(EntityAction* action, const char* effect, float time, vec3 localPosition)
{
	SDL_assert(action->numEvents < MAX_ACTION_EVENTS);
	ActionEvent* actionEvent = &action->events[action->numEvents++];
	*actionEvent = { };
	actionEvent->time = time;
	actionEvent->effectPath = effect;
	actionEvent->effectPosition = localPosition;
	actionEvent->triggered = false;
}

void StartAction(EntityAction* action, Entity* entity)
{
	RunEntityActionFunc(Start);
}

void StopAction(EntityAction* action, Entity* entity)
{
	RunEntityActionFunc(Stop);
}

void UpdateAction(EntityAction* action, Entity* entity, float deltaTime)
{
	action->elapsedTime += deltaTime;

	Creature* creature = &entity->creature;

	for (int i = 0; i < action->numEvents; i++)
	{
		ActionEvent* event = &action->events[i];
		if (!event->triggered && action->elapsedTime >= event->time)
		{
			if (event->sound)
			{
				uint32_t handle = PlaySound(event->sound, entity->position, event->volume);
				float variation = remap(game->random.nextFloat(), 0, 1, 0.9f, 1.1f);
				SetSoundRelativeSpeed(handle, event->speed * variation);
			}
			if (event->effectPath)
			{
				ParticleEffect* effect = (ParticleEffect*)CreateEntity();
				mat4 transform = GetRightWeaponTransform(creature) * mat4::Translate(event->effectPosition);
				LoadParticleEffect(effect, event->effectPath, transform.translation(), transform.rotation());
				effect->destroyOnFinish = true;
				event->effect = effect;
			}
			event->triggered = true;
		}
		if (event->triggered && event->effect)
		{
			event->effect->position = GetRightWeaponTransform(creature) * event->effectPosition;
		}
	}

	RunEntityActionFunc(Update);
}


void InitActionManager(EntityActionManager& actions, Model* moveset)
{
	actions.moveset = moveset;
	InitQueue(actions.actions);
}

static void StartActionInternal(EntityActionManager& actions, EntityAction* action, Entity* entity)
{
	action->startTime = gameTime;
	action->elapsedTime = 0;

	InitAnimation(&action->anim, action->animName, action->animMoveset ? action->animMoveset : actions.moveset, 1.0f, false, false);
	if (!action->duration)
		action->duration = action->anim.animation->duration;
	action->speed = action->speed;

	StartAction(action, entity);
}

static void StopActionInternal(EntityActionManager& actions, EntityAction* action, Entity* entity)
{
	StopAction(action, entity);
}

void QueueAction(EntityActionManager& actions, const EntityAction& action, Entity& entity)
{
	if (actions.actions.size < actions.actions.capacity)
	{
		QueuePush(actions.actions, action);
		if (actions.actions.size == 1)
			StartActionInternal(actions, QueuePeek(actions.actions), &entity);
	}
}

void CancelAction(EntityActionManager& actions, Entity& entity)
{
	if (EntityAction* currentAction = QueuePeek(actions.actions))
	{
		StopActionInternal(actions, currentAction, &entity);
		QueuePop(actions.actions);
		currentAction = QueuePeek(actions.actions);
	}
}

void UpdateActionManager(EntityActionManager& actions, Entity& entity)
{
	if (actions.actions.size > 0)
	{
		EntityAction* currentAction = QueuePeek(actions.actions);
		if (currentAction->startTime > 0)
		{
			bool shouldFinish = currentAction->elapsedTime >= currentAction->duration ||
				currentAction->followUpCancelTime && currentAction->elapsedTime >= currentAction->followUpCancelTime && actions.actions.size > 1 && currentAction->type == QueuePeekAt(actions.actions, 1)->type;
			if (shouldFinish)
			{
				StopActionInternal(actions, currentAction, &entity);
				QueuePop(actions.actions);
				currentAction = QueuePeek(actions.actions);
			}
		}

		if (currentAction)
		{
			if (!currentAction->startTime)
				StartActionInternal(actions, currentAction, &entity);
			UpdateAction(currentAction, &entity, deltaTime);
		}
	}
}
