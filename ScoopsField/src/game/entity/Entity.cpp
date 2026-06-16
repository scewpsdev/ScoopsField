#include "Entity.h"


void InitEntity(Entity* entity, EntityType type)
{
	entity->type = type;
	entity->scale = vec3(1);
}

void DestroyEntity(Entity* entity)
{
	for (int i = 0; i < entity->numDestroyCallbacks; i++)
	{
		OnEntityDestroyed(entity->destroyCallbacks[i], entity);
	}

	switch (entity->type)
	{
	case ENTITY_TYPE_CREATURE:
		DestroyCreature(&entity->creature);
		break;
	case ENTITY_TYPE_ITEM:
		DestroyItemEntity(&entity->item);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		DestroyRestingSpot(&entity->restingSpot, entity);
		break;
	case ENTITY_TYPE_PROJECTILE:
		DestroyProjectile(&entity->projectile, entity);
		break;
	case ENTITY_TYPE_TRAIL:
		DestroyTrail(&entity->trail);
		break;
	case ENTITY_TYPE_PARTICLE_EFFECT:
		DestroyParticleEffect(&entity->particles);
		break;
	case ENTITY_TYPE_RAGDOLL:
		DestroyRagdoll(&entity->ragdoll);
		break;
	case ENTITY_TYPE_SCONCE:
		DestroySconce(&entity->sconce);
		break;
	case ENTITY_TYPE_ELEVATOR:
		DestroyElevator(&entity->elevator);
		break;
	default:
		break;
	}

	SDL_memset(entity, 0, sizeof(Entity));
}

void AddDestroyCallback(Entity* entity, Entity* callbackEntity)
{
	SDL_assert(entity->numDestroyCallbacks < MAX_DESTROY_CALLBACKS);

	entity->destroyCallbacks[entity->numDestroyCallbacks++] = callbackEntity;
}

bool HitEntity(Entity* entity, HitParams* hit, Entity* by)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_CREATURE:
		return HitCreature(&entity->creature, hit, by);
	default:
		return false;
	}
}

bool InteractEntity(Entity* entity, Entity* by)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_ITEM:
		return InteractItemEntity(&entity->item, by);
	case ENTITY_TYPE_RESTING_SPOT:
		return InteractRestingSpot(&entity->restingSpot, entity, by);
	case ENTITY_TYPE_SCONCE:
		return InteractSconce(&entity->sconce, by);
	default:
		return false;
	}
}

void OnEntityDestroyed(Entity* entity, Entity* destroyed)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_PROJECTILE:
		return OnEntityDestroyed(&entity->projectile, destroyed);
	default:
		return;
	}
}

void UpdateEntity(Entity* entity)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_CREATURE:
		UpdateCreature(&entity->creature);
		break;
	case ENTITY_TYPE_ITEM:
		UpdateItemEntity(&entity->item);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		UpdateRestingSpot(&entity->restingSpot, entity);
		break;
	case ENTITY_TYPE_PROJECTILE:
		UpdateProjectile(&entity->projectile);
		break;
	case ENTITY_TYPE_TRAIL:
		UpdateTrail(&entity->trail);
		break;
	case ENTITY_TYPE_RAGDOLL:
		UpdateRagdoll(&entity->ragdoll);
		break;
	case ENTITY_TYPE_SCONCE:
		UpdateSconce(&entity->sconce);
		break;
	case ENTITY_TYPE_ELEVATOR:
		UpdateElevator(&entity->elevator);
		break;
	default:
		break;
	}
}

void RenderEntity(Entity* entity)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_CREATURE:
		RenderCreature(&entity->creature);
		break;
	case ENTITY_TYPE_ITEM:
		RenderItemEntity(&entity->item);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		RenderRestingSpot(&entity->restingSpot, entity);
		break;
	case ENTITY_TYPE_PROJECTILE:
		RenderProjectile(&entity->projectile);
		break;
	case ENTITY_TYPE_TRAIL:
		RenderTrail(&entity->trail);
		break;
	case ENTITY_TYPE_RAGDOLL:
		RenderRagdoll(&entity->ragdoll);
		break;
	case ENTITY_TYPE_SCONCE:
		RenderSconce(&entity->sconce);
		break;
	case ENTITY_TYPE_ELEVATOR:
		RenderElevator(&entity->elevator);
		break;
	default:
		break;
	}
}
