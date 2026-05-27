#include "ShootAction.h"

#include "Action.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


void InitShootAction(Action* action, Item* weapon)
{
	InitAction(action, ACTION_TYPE_SHOOT);

	action->rightAnimName = "shoot";
	action->rightAnimMoveset = &weapon->moveset;
	action->rightAnimBlendDuration = 0.0f;

	if (weapon->twoHanded)
	{
		action->leftAnimName = "shoot";
		action->leftAnimMoveset = &weapon->moveset;
		action->leftAnimBlendDuration = 0.0f;
	}

	action->animationSpeed = 1.0f;
	action->followUpCancelTime = 14 / 24.0f;

	AddActionSound(action, &game->items.bowShootSound, 0, 3, 1, 0.1f);

	//AddActionSound(action, game->swingSounds, 3, attack->damageWindow.x, 1, (attackIdx % 2 * -2 + 1) * 0.2f);
}

void StartShootAction(Action* action, Player* player)
{
	//player->stamina -= action->attack.attack->staminaCost;

	Entity* projectile = PoolAlloc(&game->entities);
	mat4 transform = GetLeftWeaponTransform(player);
	InitArrow(projectile, game->cameraPosition, game->cameraRotation.forward(), transform);
}

void StopShootAction(Action* action, Player* player)
{
}

void UpdateShootAction(Action* action, Player* player)
{
	//action->moveSpeed = clamp(action->elapsedTime / action->duration * 2, 0, 1);
}
