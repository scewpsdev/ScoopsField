#include "Projectile.h"

#include "game/player/action/Action.h"


void InitProjectile(Entity* entity, vec3 position, quat rotation)
{
	InitEntity(entity, ENTITY_TYPE_PROJECTILE);
	entity->position = position;
	entity->rotation = rotation;
	entity->model = GetModel("items/arrow/arrow.glb");
}

void DestroyProjectile(Projectile* item, Entity* entity)
{

}

bool InteractProjectile(Projectile* item, Entity* entity, Entity* by)
{
	return false;
}

void UpdateProjectile(Projectile* item, Entity* entity)
{

}

void RenderProjectile(Projectile* item, Entity* entity)
{
	RenderModel(&game->renderer, entity->model, nullptr, ModelMatrix(entity));
}
