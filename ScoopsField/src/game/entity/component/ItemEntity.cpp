#include "ItemEntity.h"

#include "game/entity/Entity.h"

#include "game/player/Player.h"

#include "renderer/Renderer.h"


void InitItemEntity(Entity* entity, Item* actualItem, vec3 position, quat rotation)
{
	InitEntity(entity, ENTITY_TYPE_ITEM);

	entity->position = position;

	entity->item.hasValue = true;
	ItemEntity* item = &entity->item.value;
	item->item = actualItem;

	item->rotation = rotation;
	InitRigidBody(&item->body, RIGID_BODY_DYNAMIC, entity->position, rotation);
	item->body.userPtr = entity;

	AABB boundingBox = item->item->model.boundingBox;
	vec3 size = boundingBox.max - boundingBox.min;
	vec3 center = (boundingBox.min + boundingBox.max) * 0.5f;
	size = max(size, vec3(0.02f));
	AddBoxCollider(&item->body, size, center, quat::Identity, ENTITY_FILTER_ITEM | ENTITY_FILTER_INTERACTABLE, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ITEM, false);
	//AddBoxCollider(&item->body, vec3(0.3f), vec3(0), quat::Identity, ENTITY_FILTER_ITEM | ENTITY_FILTER_INTERACTABLE, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ITEM, false);
}

void DestroyItemEntity(ItemEntity* item, Entity* entity)
{
	DestroyRigidBody(&item->body);
}

bool InteractItemEntity(ItemEntity* item, Entity* entity, Entity* by)
{
	if (by->type == ENTITY_TYPE_PLAYER)
	{
		Player* player = (Player*)by;

		SDL_assert(!GetCurrentAction(player));

		Action pickupAction;
		InitPickUpAction(&pickupAction, item->item);
		QueueAction(player->actions, pickupAction, *player);

		if (GiveItem(player, item->item))
		{
			entity->removed = true;
			return true;
		}
	}
	return false;
}

void UpdateItemEntity(ItemEntity* item, Entity* entity)
{
	GetRigidBodyTransform(&item->body, &entity->position, &item->rotation);
}

void RenderItemEntity(ItemEntity* item, Entity* entity)
{
	mat4 transform = mat4::Translate(entity->position) * mat4::Rotate(item->rotation);

	RenderModel(&game->renderer, &item->item->model, nullptr, transform);
}
