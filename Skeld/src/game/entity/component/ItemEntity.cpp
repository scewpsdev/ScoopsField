#include "ItemEntity.h"

#include "game/entity/Entity.h"

#include "game/player/Player.h"

#include "renderer/Renderer.h"


void InitItemEntity(ItemEntity* item, Item* actualItem, vec3 position, quat rotation)
{
	InitEntity((Entity*)item, ENTITY_TYPE_ITEM);

	item->position = position;
	item->rotation = rotation;

	item->item = actualItem;

	InitRigidBody(&item->body, RIGID_BODY_DYNAMIC, item->position, rotation, item);

	AABB boundingBox = item->item->model.boundingBox;
	vec3 size = boundingBox.max - boundingBox.min;
	vec3 center = (boundingBox.min + boundingBox.max) * 0.5f;
	size = max(size, vec3(0.02f));
	AddBoxCollider(&item->body, size, center, quat::Identity, ENTITY_FILTER_ITEM | ENTITY_FILTER_INTERACTABLE, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ITEM, false);
	//AddBoxCollider(&item->body, vec3(0.3f), vec3(0), quat::Identity, ENTITY_FILTER_ITEM | ENTITY_FILTER_INTERACTABLE, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ITEM, false);
}

void DestroyItemEntity(ItemEntity* item)
{
	DestroyRigidBody(&item->body);
}

bool InteractItemEntity(ItemEntity* item, Entity* by)
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
			item->removed = true;
			return true;
		}
	}
	return false;
}

void UpdateItemEntity(ItemEntity* item)
{
	GetRigidBodyTransform(&item->body, &item->position, &item->rotation);
}

void RenderItemEntity(ItemEntity* item)
{
	RenderModel(&game->renderer, &item->item->model, nullptr, ModelMatrix((Entity*)item));
}
