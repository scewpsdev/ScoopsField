#include "Action.h"


extern float gameTime;
extern float deltaTime;


void InitAction(Action* action, ActionType type)
{
	*action = {};
	action->type = type;
	action->speed = 1.0f;
}


void InitActionManager(ActionManager& actions, Model* moveset)
{
	actions.moveset = moveset;
	InitQueue(actions.actions);
}

static void StartActionInternal(ActionManager& actions, Action* action, Player* player)
{
	action->startTime = gameTime;

	InitAnimation(&action->anim, action->animName, action->animMoveset ? action->animMoveset : actions.moveset);
	if (!action->duration)
		action->duration = action->anim.animation->duration;
	action->speed = action->speed;

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
				currentAction->followUpCancelTime && currentAction->elapsedTime >= currentAction->followUpCancelTime && actions.actions.size > 1 && currentAction->type == QueuePeekAt(actions.actions, 1)->type;
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
