#include "Entity.h"


void InitEntity(Entity* entity, EntityType type)
{
	SDL_memset(entity, 0, sizeof(Entity));

	entity->type = type;
}

void DestroyEntity(Entity* entity)
{
	if (entity->creature)
		DestroyCreature(&entity->creature.value, entity);
	if (entity->item)
		DestroyItemEntity(&entity->item.value, entity);

	SDL_memset(entity, 0, sizeof(Entity));
}

bool HitEntity(Entity* entity, HitParams* hit, Entity* by)
{
	if (entity->creature)
		return HitCreature(&entity->creature.value, entity, hit, by);
	return false;
}

bool InteractEntity(Entity* entity, Entity* by)
{
	if (entity->item)
		return InteractItemEntity(&entity->item.value, entity, by);
	return false;
}

void UpdateEntity(Entity* entity)
{
	if (entity->creature)
		UpdateCreature(&entity->creature.value, entity);
	if (entity->item)
		UpdateItemEntity(&entity->item.value, entity);
}

void RenderEntity(Entity* entity)
{
	if (entity->creature)
		RenderCreature(&entity->creature.value, entity);
	if (entity->item)
		RenderItemEntity(&entity->item.value, entity);
}
