#pragma once

#include "math/Vector.h"

#include "utils/Optional.h"

#include "component/Creature.h"
#include "component/ItemEntity.h"
#include "component/RestingSpot.h"


enum EntityPhysicsFilter
{
	ENTITY_FILTER_DEFAULT = 1 << 0,
	ENTITY_FILTER_ENEMY = 1 << 1,
	ENTITY_FILTER_PLAYER = 1 << 2,
	ENTITY_FILTER_ITEM = 1 << 3,
	ENTITY_FILTER_INTERACTABLE = 1 << 4,
};

enum EntityType
{
	ENTITY_TYPE_NONE = 0,

	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_SKELETON,
	ENTITY_TYPE_ITEM,
	ENTITY_TYPE_RESTING_SPOT,

	ENTITY_TYPE_LAST
};

struct HitParams
{
	int damage = 1;
	float damageMultiplier = 1;
	vec3 position;
};

struct Entity;

struct Entity
{
	EntityType type;
	bool removed;

	// transform
	vec3 position;
	quat rotation;

	Model* model;

	union {
		Creature creature;
		ItemEntity item;
		RestingSpot restingSpot;
	};
};


void InitEntity(Entity* entity, EntityType type);
void DestroyEntity(Entity* entity);

bool HitEntity(Entity* entity, HitParams* hit, Entity* by);
bool InteractEntity(Entity* entity, Entity* by);

void UpdateEntity(Entity* entity);
void RenderEntity(Entity* entity);

inline mat4 ModelMatrix(Entity* entity)
{
	return mat4::Translate(entity->position) * mat4::Rotate(entity->rotation);
}
