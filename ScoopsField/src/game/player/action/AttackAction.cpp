#include "AttackAction.h"

#include "Application.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


#define HIT_FREEZE_DURATION 0.1f


void InitAttackAction(Action* action, Item* weapon, Attack* attack, int attackIdx, uint32_t button, uint32_t cancelButton)
{
	InitAction(action, ACTION_TYPE_ATTACK);

	action->rightAnimName = attack->animation;
	action->rightAnimMoveset = &weapon->moveset;

	if (attack->bowDraw)
	{
		action->overrideLeftWeapon = true;
		action->leftWeapon = &game->items.items[ITEM_ARROW];

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
	//action->rootMotion = true;

	action->attack.weapon = weapon;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	action->attack.button = button;
	action->attack.cancelButton = cancelButton;

	if (attack->stance)
	{
		action->duration = 1000;
		action->idleAnimStrength = 0.5f;
	}

	for (int i = 0; i < attack->numSounds; i++)
	{
		AddActionSound(action, attack->sounds[i].sound, attack->sounds[i].time, attack->sounds[i].volume, attack->sounds[i].speed, attack->sounds[i].pan);
	}
	for (int i = 0; i < attack->numEffects; i++)
	{
		AddActionEffect(action, attack->effects[i].path, attack->effects[i].time, attack->effects[i].localPosition);
	}

	InitList(&action->attack.hitEntities);
}

void StartAttackAction(Action* action, Player* player)
{
	player->stamina -= action->attack.attack->staminaCost;
}

void StopAttackAction(Action* action, Player* player)
{
	if (action->attack.attack->bowDraw && !action->attack.cancelled)
	{
		float power = min(action->elapsedTime / action->followUpCancelTime, 1.0f);

		Action shootAction = {};
		InitShootAction(&shootAction, action->attack.weapon, power);
		ClearQueuedAction(player->actions);
		QueueAction(player->actions, shootAction, *player);
	}

	player->blockItem = nullptr;
	player->parry = false;
}

void UpdateAttackAction(Action* action, Player* player)
{
	action->animationSpeed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);
	action->rightAnim.speed = action->animationSpeed;
	//action->speed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);

	if (action->elapsedTime >= action->attack.attack->blockWindow.x && action->elapsedTime <= action->attack.attack->blockWindow.y)
		player->blockItem = action->attack.weapon;
	else
		player->blockItem = nullptr;

	player->parry = action->elapsedTime >= action->attack.attack->parryWindow.x && action->elapsedTime <= action->attack.attack->parryWindow.y;

	if (action->attack.attack->stance)
	{
		bool parry = action->elapsedTime <= action->attack.attack->parryWindow.y;
		bool blockStagger = gameTime - player->lastBlockTime < GUARD_BREAK_STAGGER_DURATION && player->lastBlockStagger;

		action->moveSpeed = action->attack.attack->bowDraw || parry || blockStagger ? 0.3f : player->blockItem ? 0.7f : 1.0f;

		if (action->attack.button && !GetMouseButton(action->attack.button) && action->elapsedTime > action->followUpCancelTime)
			CancelAction(player->actions, *player);
		if (action->attack.attack->bowDraw && action->attack.cancelButton && GetMouseButton(action->attack.cancelButton))
		{
			action->attack.cancelled = true;
			CancelAction(player->actions, *player);
		}
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
			int numHits = Raycast(origin, direction, range, hits, 16, ENTITY_FILTER_ENEMY_HITBOX);
			for (int i = 0; i < numHits; i++)
			{
				PhysicsHit* hit = &hits[i];
				Entity* hitEntity = (Entity*)hit->body->userPtr;

				if (!action->attack.hitEntities.contains(hitEntity))
				{
					HitParams params = {};
					params.damage = action->attack.weapon->weapon.damage * action->attack.attack->damageMultiplier;
					params.damageType = action->attack.attack->damageType;
					params.position = hit->position;
					params.body = hit->body;
					params.impulse = direction * 0.1f;

					if (HitEntity(hitEntity, &params, (Entity*)player))
					{
						action->attack.lastHitTime = gameTime;

						//game->points += 10;

						PlaySound(&game->hitSlashSound, hit->position);
					}

					action->attack.hitEntities.add(hitEntity);
				}
			}

			vec3 mid = origin + 0.5f * range * direction;

			if (!action->attack.trail)
			{
				action->attack.trail = (Trail*)CreateEntity();
				InitTrail(action->attack.trail, mid, false, 8);
				action->attack.trail->texture = GetTexture("textures/effect/trail_weapon.png");
				action->attack.trail->color = vec4(1);
				action->attack.trail->billboard = false;
				action->attack.trail->fadeAlpha = true;
			}

			action->attack.trail->position = mid;
			action->attack.trail->rotation = quat::FromAxes(direction, vec3::Up);
			action->attack.trail->width = range;
		}
		else
		{
			if (action->attack.trail)
			{
				action->attack.trail->destroyOnCollapse = true;
				action->attack.trail = nullptr;
			}
		}
	}

	if (action->attack.attack->projectileCast && action->elapsedTime >= action->attack.attack->projectileCastTime && !action->attack.projectile)
	{
		Projectile* projectile = (Projectile*)PoolAlloc(&game->entities);
		mat4 transform = GetRightWeaponTransform(player) * mat4::Translate(action->attack.weapon->weapon.castOffset);
		InitMagicProjectile(projectile, game->cameraPosition, game->cameraRotation.forward(), transform, (Entity*)player);
		action->attack.projectile = projectile;
	}

	if (action->attack.projectile && action->attack.projectile->trail)
		BendTrailEnd(action->attack.projectile->trail, (GetRightWeaponTransform(player) * mat4::Translate(action->attack.weapon->weapon.castOffset)).translation(), 2);
}
