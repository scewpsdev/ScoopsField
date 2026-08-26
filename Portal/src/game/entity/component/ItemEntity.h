#pragma once

#include "game/entity/EntityBase.h"

#include "physics/RigidBody.h"


struct Entity;

struct ItemEntity : EntityBase
{
	Item* item;

	RigidBody body;
};


void InitItemEntity(ItemEntity* item, Item* actualItem, vec3 position, quat rotation);
void DestroyItemEntity(ItemEntity* item);

bool InteractItemEntity(ItemEntity* item, Entity* by);

void UpdateItemEntity(ItemEntity* item);
void RenderItemEntity(ItemEntity* item);
