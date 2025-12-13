#include "Action.h"


extern float gameTime;
extern float deltaTime;


void InitAnimation(AnimationPlayback* animation, const char* name, Model* moveset, float speed, bool loop, bool mirror)
{
	SDL_strlcpy(animation->name, name, 32);
	animation->speed = speed;
	animation->loop = loop;
	animation->mirror = mirror;
	animation->animation = GetAnimationByName(moveset, name);
}


void InitAction(Action* action, ActionType type)
{
	*action = {};
	action->type = type;
}


void InitActionManager(ActionManager& actions)
{
	InitQueue(actions.actions);
}

static void StartAction(ActionManager& actions, Action* action)
{
	action->startTime = gameTime;

	InitAnimation(&action->anim, action->animName, action->animMoveset);
	action->duration = action->anim.animation->duration;
}

static void StopAction(ActionManager& actions, Action* action)
{

}

void QueueAction(ActionManager& actions, const Action& action)
{
	if (actions.actions.size < actions.actions.capacity)
	{
		QueuePush(actions.actions, action);
		if (actions.actions.size == 1)
			StartAction(actions, QueuePeek(actions.actions));
	}
}

void UpdateActionManager(ActionManager& actions)
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
				StopAction(actions, currentAction);
				QueuePop(actions.actions);
				currentAction = QueuePeek(actions.actions);
			}
		}

		if (currentAction)
		{
			if (!currentAction->startTime)
				StartAction(actions, currentAction);
			UpdateAction(currentAction, deltaTime);
		}
	}
}
