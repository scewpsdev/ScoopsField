#include "EntityAction.h"


extern float gameTime;
extern float deltaTime;


void InitAction(EntityAction* action, EntityActionType type)
{
	*action = {};
	action->type = type;
	action->speed = 1.0f;
	action->walkSpeed = 1.0f;
}


void InitActionManager(EntityActionManager& actions, Model* moveset)
{
	actions.moveset = moveset;
	InitQueue(actions.actions);
}

static void StartActionInternal(EntityActionManager& actions, EntityAction* action, Entity* entity)
{
	action->startTime = gameTime;

	InitAnimation(&action->anim, action->animName, action->animMoveset ? action->animMoveset : actions.moveset);
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
