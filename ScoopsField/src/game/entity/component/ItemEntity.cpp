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
	AddBoxCollider(&item->body, vec3(0.3f), vec3(0), quat::Identity, ENTITY_FILTER_ITEM | ENTITY_FILTER_INTERACTABLE, ENTITY_FILTER_DEFAULT, false);
	item->body.userPtr = entity;
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
		if (GiveItem(player, item->item))
		{
			// pick up action
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
