#include "EquipAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitSitAction(Action* action)
{
	InitAction(action, ACTION_TYPE_SIT);

	action->rightAnimName = "sit";
	action->rightAnimMoveset = nullptr;

	action->leftAnimName = "sit";
	action->leftAnimMoveset = nullptr;

	action->bodyAnimName = "sit_";
	action->bodyAnimMoveset = nullptr;

	action->duration = 10;
}

void StartSitAction(Action* action, Player* player)
{
}

void StopSitAction(Action* action, Player* player)
{
}

void UpdateSitAction(Action* action, Player* player)
{
}
