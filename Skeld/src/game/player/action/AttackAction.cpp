#include "AttackAction.h"

#include "Core.h"
#include "Resource.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


#define HIT_FREEZE_DURATION 0.1f


void InitAttackAction(Action* action, Item* weapon, bool right, Attack* attack, int attackIdx, uint32_t button, uint32_t cancelButton)
{
	InitAction(action, ACTION_TYPE_ATTACK);

	action->duration = 1.0f;
	action->controlWeaponTransform = true;

	AddActionSound(action, &game->swingSound, 0.4f * action->duration, 0.5f, 1, 0);

	/*
	if (right)
	{
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
	}
	else
	{
		action->leftAnimName = attack->animation;
		action->leftAnimMoveset = &weapon->moveset;
		action->leftAnimMirror = true;

		if (attack->bowDraw)
		{
			action->overrideRightWeapon = true;
			action->rightWeapon = &game->items.items[ITEM_ARROW];

			action->leftAnimBlendDuration = 0.0f;

			if (attack->twoHanded)
				action->rightAnimBlendDuration = 0.0f;
		}

		if (attack->twoHanded)
		{
			action->rightAnimName = attack->animation;
			action->rightAnimMoveset = &weapon->moveset;
			action->rightAnimMirror = true;
		}

		if (attack->itemAnimation)
		{
			action->leftItemAnimName = attack->itemAnimation;
			action->leftItemAnimMoveset = &weapon->model;
			action->leftItemAnimBlendDuration = 0.0f;
		}
	}
	*/

	//action->animationSpeed = attack->animationSpeed;
	action->moveSpeed = 0.5f;
	//action->followUpCancelTime = attack->followUpCancelTime;
	//action->rootMotion = true;

	action->attack.weapon = weapon;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	action->attack.button = button;
	action->attack.cancelButton = cancelButton;

	/*
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
	*/

	InitList(&action->attack.hitEntities);
}

void StartAttackAction(Action* action, Player* player)
{
	//player->stamina -= action->attack.attack->staminaCost;
	player->weaponTransformInterpolator = 0;
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

	if (action->attack.trail)
	{
		action->attack.trail->destroyOnCollapse = true;
		action->attack.trail = nullptr;
	}

	player->weaponTransformInterpolator = 0;
}

void UpdateAttackAction(Action* action, Player* player)
{
	//action->animationSpeed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && !action->attack.lastHitReflect && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);
	//action->actionSpeed = action->animationSpeed;
	//action->rightAnim.speed = action->animationSpeed;
	if (action->attack.lastHitTime && action->attack.lastHitReflect)
	{
		action->animationSpeed *= -0.5f;
	}
	//action->speed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);

	//if (action->elapsedTime >= action->attack.attack->blockWindow.x && action->elapsedTime <= action->attack.attack->blockWindow.y)
	//	player->blockItem = action->attack.weapon;
	//else
	//	player->blockItem = nullptr;

	//player->parry = action->elapsedTime >= action->attack.attack->parryWindow.x && action->elapsedTime <= action->attack.attack->parryWindow.y;

	//mat4 weaponTransform = GetRightWeaponTransform(player);
	quat weaponRotation = quat::FromAxisAngle(vec3::Right, -0.5f * PI) * quat::FromAxisAngle(vec3::Up, 0.5f * PI);
	float range = 1.0f;
	vec3 weaponTranslation = vec3(0, 0, -range + action->attack.weapon->weapon.damageRange.y);
	mat4 weaponTransform = mat4::Transform(weaponTranslation, weaponRotation);
	float tilt = 30;
	float angle = action->elapsedTime / action->duration;
	if (action->attack.lastHitReflect)
	{
		angle = -(angle - 0.5f);
		angle *= 0.2f;
		//angle = sign(angle) * SDL_powf(SDL_fabsf(angle) * 2, 0.5f) / 2;
		angle += 0.5f;

		tilt = -20;
	}
	angle = smoothstep(0.3f, 0.7f, angle);
	angle = (angle - 0.5f) * PI;
	weaponTransform = mat4::Rotate(vec3::Up, angle) * weaponTransform;
	weaponTransform = mat4::Rotate(vec3::Back, tilt * Deg2Rad) * weaponTransform;
	weaponTransform = mat4::Translate(0, -0.2f, 0) * weaponTransform;
	action->weaponTransform = weaponTransform;

	if (action->elapsedTime / action->duration >= 0.5f && !action->attack.didRaycast)
	{
		action->attack.didRaycast = true;

		PhysicsHit hits[16];
		int numHits = Raycast(game->cameraPosition, game->cameraRotation.forward(), range, hits, 16, ENTITY_FILTER_ENEMY_HITBOX);
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
				//params.force = (tip - action->attack.lastHitboxTip).normalized() * 0.1f;
				params.force = game->cameraRotation.left();

				if (HitEntity(hitEntity, &params, (Entity*)player))
				{
					action->attack.lastHitTime = gameTime;

					if (params.wasBlocked)
					{
						action->attack.lastHitReflect = true;

						action->attack.trail->destroyOnCollapse = true;
						action->attack.trail = nullptr;
					}

					//game->points += 10;

					//PlaySound(&game->hitSlashSound, hit->position);
				}

				action->attack.hitEntities.add(hitEntity);
			}
		}
	}

	mat4 cameraTransform = mat4::Transform(game->cameraPosition, game->cameraRotation);
	vec3 mid = cameraTransform * (weaponTransform.translation() + weaponTransform.rotation().up() * 0.5f * action->attack.weapon->weapon.damageRange.y);

	if (!action->attack.trail && !action->attack.lastHitTime)
	{
		action->attack.trail = (Trail*)CreateEntity();
		InitTrail(action->attack.trail, mid, false, 8);
		action->attack.trail->texture = GetTexture("textures/effect/trail_weapon.png");
		action->attack.trail->color = vec4(1, 1, 1, 0.1f);
		action->attack.trail->billboard = false;
		action->attack.trail->fadeAlpha = true;
	}

	if (action->attack.trail)
	{
		action->attack.trail->position = mid;
		action->attack.trail->rotation = (cameraTransform * weaponTransform).rotation() * quat::FromAxisAngle(vec3::Up, PI) * quat::FromAxisAngle(vec3::Back, 0.5f * PI);
		action->attack.trail->width = action->attack.weapon->weapon.damageRange.y;
	}

	//vec3 direction = weaponTransform.rotation().up();
	//vec3 origin = weaponTransform.translation() + action->attack.weapon->weapon.damageRange.x * direction;
	//float range = action->attack.weapon->weapon.damageRange.y - action->attack.weapon->weapon.damageRange.x;
	//vec3 tip = origin + direction * range;

	/*
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
					params.force = (tip - action->attack.lastHitboxTip).normalized() * 0.1f;

					if (HitEntity(hitEntity, &params, (Entity*)player))
					{
						action->attack.lastHitTime = gameTime;

						if (params.wasBlocked)
							action->attack.lastHitReflect = true;

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

	action->attack.lastHitboxTip = tip;
	*/

	if (action->attack.attack->resetHitboxTime && action->elapsedTime >= action->attack.attack->resetHitboxTime && !action->attack.resetHitbox)
	{
		action->attack.hitEntities.clear();
		action->attack.resetHitbox = true;
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
