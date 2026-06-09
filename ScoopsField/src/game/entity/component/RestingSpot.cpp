#include "RestingSpot.h"

#include "game/player/action/Action.h"


void InitRestingSpot(Entity* entity, vec3 position, quat rotation)
{
	InitEntity(entity, ENTITY_TYPE_RESTING_SPOT);
	entity->position = position;
	entity->rotation = rotation;
	entity->model = GetModel("entities/object/carpet/carpet.glb");

	InitRigidBody(&entity->restingSpot.body, RIGID_BODY_STATIC, entity->position, rotation, entity);

	AABB boundingBox = entity->model->boundingBox;
	vec3 size = boundingBox.max - boundingBox.min;
	vec3 center = (boundingBox.min + boundingBox.max) * 0.5f;
	size = max(size, vec3(0.02f));
	AddBoxCollider(&entity->restingSpot.body, size, center, quat::Identity, ENTITY_FILTER_INTERACTABLE, 0, false);
}

void DestroyRestingSpot(RestingSpot* restingSpot, Entity* entity)
{
	DestroyRigidBody(&restingSpot->body);
}

bool InteractRestingSpot(RestingSpot* restingSpot, Entity* entity, Entity* by)
{
	Action action = {};
	InitSitAction(&action);
	QueueAction(game->player.actions, action, game->player);
	return true;
}

void UpdateRestingSpot(RestingSpot* restingSpot, Entity* entity)
{

}

void RenderRestingSpot(RestingSpot* restingSpot, Entity* entity)
{
	RenderModel(&game->renderer, entity->model, nullptr, ModelMatrix(entity));
}
