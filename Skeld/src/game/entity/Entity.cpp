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
#define ENTITY_TYPE(CAPS, Pascal, camel) case ENTITY_TYPE_##CAPS:\
											 Destroy##Pascal(&entity->camel);\
											 break;
		SWITCH_ENTITY()
#undef ENTITY_TYPE

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
	case ENTITY_TYPE_TREE:
		return HitTree(&entity->tree, hit, by);
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
		return InteractRestingSpot(&entity->restingSpot, by);
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
#define ENTITY_TYPE(CAPS, Pascal, camel)\
	case ENTITY_TYPE_##CAPS:\
		Update##Pascal(&entity->camel);\
		break;
		SWITCH_ENTITY()
#undef ENTITY_TYPE

	default:
		break;
	}
}

void RenderEntity(Entity* entity)
{
	switch (entity->type)
	{
#define ENTITY_TYPE(CAPS, Pascal, camel)\
	case ENTITY_TYPE_##CAPS:\
		Render##Pascal(&entity->camel);\
		break;
		SWITCH_ENTITY()
#undef ENTITY_TYPE

	default:
		break;
	}
}
