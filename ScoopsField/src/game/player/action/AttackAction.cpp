#include "AttackAction.h"

#include "Application.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


#define HIT_FREEZE_DURATION 0.1f


void InitAttackAction(Action* action, Item* weapon, Attack* attack, int attackIdx, uint32_t button)
{
	InitAction(action, ACTION_TYPE_ATTACK);

	action->rightAnimName = attack->animation;
	action->rightAnimMoveset = &weapon->moveset;

	if (attack->projectileShoot)
	{
		action->overrideLeftWeapon = true;
		action->leftWeapon = &game->items.items[ITEM_TYPE_ARROW];

		action->rightAnimBlendDuration = 0.0f;

		if (attack->twoHanded)
			action->leftAnimBlendDuration = 0.0f;
	}

	if (attack->twoHanded)
	{
		action->leftAnimName = attack->animation;
		action->leftAnimMoveset = &weapon->moveset;
	}

	if (attack->itemAnimation)
	{
		action->rightItemAnimName = attack->itemAnimation;
		action->rightItemAnimMoveset = &weapon->model;
		action->rightItemAnimBlendDuration = 0.0f;
	}

	action->animationSpeed = attack->animationSpeed;
	//action->moveSpeed = 0.5f;
	action->followUpCancelTime = attack->followUpCancelTime;
	action->rootMotion = true;

	action->attack.weapon = weapon;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	action->attack.button = button;

	if (attack->stance)
	{
		action->duration = 1000;
		action->idleAnimStrength = 0.5f;
	}
	else
	{
		AddActionSound(action, &game->swingSound, attack->damageWindow.x, 1, 1, (attackIdx % 2 * -2 + 1) * 0.2f);
	}

	for (int i = 0; i < attack->numSounds; i++)
	{
		AddActionSound(action, attack->sounds[i].sound, attack->sounds[i].time, attack->sounds[i].volume, attack->sounds[i].speed, attack->sounds[i].pan);
	}

	InitList(&action->attack.hitEntities);
}

void StartAttackAction(Action* action, Player* player)
{
	player->stamina -= action->attack.attack->staminaCost;
}

void StopAttackAction(Action* action, Player* player)
{
	if (action->attack.attack->projectileShoot)
	{
		Action shootAction = {};
		InitShootAction(&shootAction, action->attack.weapon);
		QueueAction(player->actions, shootAction, *player);
	}
}

void UpdateAttackAction(Action* action, Player* player)
{
	action->animationSpeed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);
	action->rightAnim.speed = action->animationSpeed;
	//action->speed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);

	if (action->attack.attack->stance)
	{
		bool parry = action->elapsedTime <= action->attack.attack->parryWindow.y;

		action->moveSpeed = action->attack.attack->projectileShoot || parry ? 0.5f : 1.0f;

		if (!GetMouseButton(action->attack.button) && action->elapsedTime > action->attack.attack->followUpCancelTime)
			CancelAction(player->actions, *player);
	}
	else
	{
		bool damage = action->elapsedTime >= action->attack.attack->damageWindow.x && action->elapsedTime <= action->attack.attack->damageWindow.y;

		action->moveSpeed = action->elapsedTime >= action->attack.attack->damageWindow.y ? 0.5f : 1.0f; // damage ? 0.5f : 1.0f;

		if (damage)
		{
			mat4 weaponTransform = GetRightWeaponTransform(player);
			vec3 direction = weaponTransform.rotation().up();
			vec3 origin = weaponTransform.translation() + action->attack.weapon->weapon.damageRange.x * direction;
			float range = action->attack.weapon->weapon.damageRange.y - action->attack.weapon->weapon.damageRange.x;

			PhysicsHit hits[16];
			int numHits = Raycast(origin, direction, range, hits, 16, ENTITY_FILTER_ENEMY);
			for (int i = 0; i < numHits; i++)
			{
				PhysicsHit* hit = &hits[i];

				if (!action->attack.hitEntities.contains(hit->body))
				{
					Entity* hitEntity = (Entity*)hit->body->userPtr;

					HitParams params = {};
					params.damage = action->attack.weapon->weapon.damage;
					params.damageMultiplier = action->attack.attack->damageMultiplier;
					params.position = hit->position;

					if (HitEntity(hitEntity, &params, player))
					{
						action->attack.lastHitTime = gameTime;

						//game->points += 10;

						PlaySound(&game->slashHitSound, hit->position);
					}

					action->attack.hitEntities.add(hit->body);
				}
			}
		}
	}
}
