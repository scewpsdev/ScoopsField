#include "TurnAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitTurnAction(Action* action, int direction)
{
	InitAction(action, ACTION_TYPE_TURN);

	action->bodyAnimName = direction > 0 ? "turn_left" : "turn_right";
	action->bodyAnimMoveset = nullptr;

	//action->fullBodyAnim = true;
}

void StartTurnAction(Action* action, Player* player)
{
}

void StopTurnAction(Action* action, Player* player)
{
}

void UpdateTurnAction(Action* action, Player* player)
{
}
