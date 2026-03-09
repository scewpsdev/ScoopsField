#include "AttackAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


#define HIT_FREEZE_DURATION 0.1f


// TODO
// [X] hit raycast
// [X] hittable object
// [X] skeleton
// [X] skeleton action queue
// [X] skeleton hit reaction
// [X] skeleton death
// [X] rounds

// [X] skeleton ai
// [X] player damage & health
// [X] testmap
// [X] navmesh
// [X] player death
// [X] player hp regen
// [X] round indicator
// [X] points
// [X] stamina
// [X] sounds
// [X] source movement
// [ ] doors
// [ ] wallbuys
// [ ] hit particles
// [ ] perks

// [ ] game over screen


void InitAttackAction(Action* action, Item* weapon, Attack* attack, int attackIdx)
{
	InitAction(action, ACTION_TYPE_ATTACK);
	action->animName = attack->animation;
	action->animMoveset = &weapon->moveset;
	action->animationSpeed = attack->animationSpeed;
	action->twoHanded = weapon->twoHanded;
	//action->moveSpeed = 0.5f;
	action->followUpCancelTime = attack->followUpCancelTime;
	action->attack.weapon = weapon;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	AddActionSound(action, game->swingSounds, 3, attack->damageWindow.x, 1, (attackIdx % 2 * -2 + 1) * 0.2f);

	InitList(&action->attack.hitEntities);
}

void StartAttackAction(Action* action, Player* player)
{
	player->stamina -= 0.1f;
}

void StopAttackAction(Action* action, Player* player)
{
}

void UpdateAttackAction(Action* action, Player* player)
{
	action->animationSpeed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);
	action->anim.speed = action->animationSpeed;
	//action->speed = action->attack.attack->animationSpeed * (action->attack.lastHitTime && gameTime - action->attack.lastHitTime < HIT_FREEZE_DURATION ? 0.2f : 1);

	bool damage = action->elapsedTime >= action->attack.attack->damageWindow.x && action->elapsedTime <= action->attack.attack->damageWindow.y;
	if (damage)
	{
		mat4 weaponTransform = GetRightWeaponTransform(player);
		vec3 direction = weaponTransform.rotation().up();
		vec3 origin = weaponTransform.translation() + action->attack.weapon->weapon.damageRange.x * direction;
		float range = action->attack.weapon->weapon.damageRange.y - action->attack.weapon->weapon.damageRange.x;

		PhysicsHit hits[16];
		int numHits = Raycast(physics, origin, direction, range, hits, 16, ENTITY_FILTER_ENEMY);
		for (int i = 0; i < numHits; i++)
		{
			PhysicsHit* hit = &hits[i];

			if (!action->attack.hitEntities.contains(hit->body))
			{
				Entity* hitEntity = (Entity*)hit->body->userPtr;
				if (hitEntity->hitCallback)
				{
					HitParams params = {};
					params.damage = action->attack.weapon->weapon.damage;
					params.damageMultiplier = action->attack.attack->damageMultiplier;
					params.position = hit->position;
					hitEntity->hitCallback(hitEntity, params, player);

					action->attack.lastHitTime = gameTime;
				}

				game->points += 10;

				PlaySound(&game->slashHitSounds[game->random.next() % 2], hit->position);

				action->attack.hitEntities.add(hit->body);
			}
		}
	}
}
