#include "AttackAction.h"

#include "Action.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/player/Player.h"
#include "game/entity/Entity.h"


// TODO
// [X] hit raycast
// [X] hittable object
// [X] skeleton
// [X] skeleton action queue
// [X] skeleton hit reaction
// [X] skeleton death
// [X] rounds
// [ ] skeleton attack
// [ ] player health
// [ ] respawn


void InitAttackAction(Action* action, Item* weapon, Attack* attack, int attackIdx)
{
	InitAction(action, ACTION_TYPE_ATTACK);
	action->animName = attack->animation;
	action->animMoveset = &weapon->moveset;
	action->followUpCancelTime = attack->followUpCancelTime;
	action->attack.weapon = weapon;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	InitList(&action->attack.hitEntities);
}

void StartAttackAction(Action* action, Player* player)
{
}

void StopAttackAction(Action* action, Player* player)
{
}

void UpdateAttackAction(Action* action, Player* player)
{
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
					hitEntity->hitCallback(hitEntity, params, player);
				}

				action->attack.hitEntities.add(hit->body);
			}
		}
	}
}
