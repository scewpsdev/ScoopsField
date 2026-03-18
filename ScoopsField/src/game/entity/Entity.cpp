#include "Entity.h"


void InitEntity(Entity* entity, EntityType type)
{
	SDL_memset(entity, 0, sizeof(Entity));

	entity->type = type;
}

void DestroyEntity(Entity* entity)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_SKELETON:
		DestroyCreature(&entity->creature, entity);
		break;
	case ENTITY_TYPE_ITEM:
		DestroyItemEntity(&entity->item, entity);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		DestroyRestingSpot(&entity->restingSpot, entity);
		break;
	default:
		break;
	}

	SDL_memset(entity, 0, sizeof(Entity));
}

bool HitEntity(Entity* entity, HitParams* hit, Entity* by)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_SKELETON:
		return HitCreature(&entity->creature, entity, hit, by);
	default:
		return false;
	}
}

bool InteractEntity(Entity* entity, Entity* by)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_ITEM:
		return InteractItemEntity(&entity->item, entity, by);
	case ENTITY_TYPE_RESTING_SPOT:
		return InteractRestingSpot(&entity->restingSpot, entity, by);
	default:
		return false;
	}
}

void UpdateEntity(Entity* entity)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_SKELETON:
		UpdateCreature(&entity->creature, entity);
		break;
	case ENTITY_TYPE_ITEM:
		UpdateItemEntity(&entity->item, entity);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		UpdateRestingSpot(&entity->restingSpot, entity);
		break;
	default:
		break;
	}
}

void RenderEntity(Entity* entity)
{
	switch (entity->type)
	{
	case ENTITY_TYPE_SKELETON:
		RenderCreature(&entity->creature, entity);
		break;
	case ENTITY_TYPE_ITEM:
		RenderItemEntity(&entity->item, entity);
		break;
	case ENTITY_TYPE_RESTING_SPOT:
		RenderRestingSpot(&entity->restingSpot, entity);
		break;
	default:
		break;
	}
}
