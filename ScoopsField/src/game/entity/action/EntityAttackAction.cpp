#include "EntityAttackAction.h"

#include "Application.h"

#include "EntityAction.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/entity/Entity.h"


void InitEntityAttackAction(EntityAction* action, EntityAttack* attack, int attackIdx)
{
	InitAction(action, ENTITY_ACTION_TYPE_ATTACK);
	action->animName = attack->animation;
	action->animMoveset = nullptr;
	action->fullBody = true;
	action->rootMotion = true;
	action->walkSpeed = 0;
	action->turnSpeed = 0;
	action->followUpCancelTime = attack->followUpCancelTime;
	action->attack.attack = attack;
	action->attack.attackIdx = attackIdx;

	InitList(&action->attack.hitEntities);
}

void StartEntityAttackAction(EntityAction* action, Entity* entity)
{
}

void StopEntityAttackAction(EntityAction* action, Entity* entity)
{
}

void UpdateEntityAttackAction(EntityAction* action, Entity* entity)
{
	Creature* creature = &entity->creature;
	bool damage = action->elapsedTime >= action->attack.attack->damageWindow.x && action->elapsedTime <= action->attack.attack->damageWindow.y;
	if (damage)
	{
		mat4 weaponTransform = ModelMatrix(entity) * GetNodeTransform(&creature->anim, creature->rightWeaponNode);
		vec3 direction = weaponTransform.rotation().up();
		vec3 origin = weaponTransform.translation();
		float range = creature->weaponRange;

		PhysicsHit hits[16];
		int numHits = SweepSphere(0.1f, origin, direction, range, hits, 16, ENTITY_FILTER_PLAYER);
		for (int i = 0; i < numHits; i++)
		{
			PhysicsHit* hit = &hits[i];
			RigidBody* body = hits[i].body;
			Entity* hitEntity = (Entity*)body->userPtr;

			if (!action->attack.hitEntities.contains(hitEntity))
			{
				HitParams params = {};
				params.damage = creature->damage;
				params.position = hit->position;
				params.body = hit->body;

				Player* player = (Player*)hitEntity;
				if (HitPlayer(player, params, entity))
				{
					PlaySound(&game->slashHitSound, hit->position);
				}

				action->attack.hitEntities.add(hitEntity);
			}
		}
	}

	bool turn = action->elapsedTime >= 0.75f * action->attack.attack->damageWindow.x && action->elapsedTime <= mix(action->attack.attack->damageWindow.x, action->attack.attack->damageWindow.y, 0.5f);
	action->turnSpeed = turn ? 2.0f : 0.0f;
}
