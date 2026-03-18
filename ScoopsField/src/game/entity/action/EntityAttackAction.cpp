#include "EntityAttackAction.h"

#include "EntityAction.h"

#include "physics/Physics.h"

#include "game/Game.h"
#include "game/entity/Entity.h"


void InitEntityAttackAction(EntityAction* action, const char* animation, int attackIdx)
{
	InitAction(action, ENTITY_ACTION_TYPE_ATTACK);
	action->animName = animation;
	action->animMoveset = nullptr;
	action->attack.weapon = nullptr;
	action->attack.attack = nullptr;
	action->attack.attackIdx = attackIdx;
	action->attack.damageRange = vec2((float)15, (float)18) / 24.0f;

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
	Creature* skeleton = &entity->creature;
	bool damage = action->elapsedTime >= action->attack.damageRange.x && action->elapsedTime <= action->attack.damageRange.y;
	if (damage)
	{
		PhysicsHit hits[16];
		int numHits = OverlapSphere(physics, entity->position + vec3::Up + quat::FromAxisAngle(vec3::Up, skeleton->lookDirection).forward() * 0.5f, 0.5f, hits, 16, ENTITY_FILTER_PLAYER);
		for (int i = 0; i < numHits; i++)
		{
			RigidBody* body = hits[i].body;
			if (!action->attack.hitEntities.contains(body))
			{
				Entity* entity = (Entity*)body->userPtr;
				if (entity && entity->type == ENTITY_TYPE_PLAYER)
				{
					Player* player = (Player*)entity;
					HitParams params = {};
					params.damage = 20;
					HitPlayer(player, params, entity);
				}
				action->attack.hitEntities.add(body);
			}
		}
	}
}
