#pragma once

#include "physics/RigidBody.h"


struct Entity;

struct ItemEntity
{
	Item* item;

	quat rotation;
	RigidBody body;
};


void InitItemEntity(Entity* entity, Item* actualItem, vec3 position, quat rotation);
void DestroyItemEntity(ItemEntity* item, Entity* entity);

bool InteractItemEntity(ItemEntity* item, Entity* entity, Entity* by);

void UpdateItemEntity(ItemEntity* item, Entity* entity);
void RenderItemEntity(ItemEntity* item, Entity* entity);
