#pragma once


enum EntityPhysicsFilter
{
	ENTITY_FILTER_DEFAULT = 1 << 0,
	ENTITY_FILTER_ENEMY = 1 << 1,
};

enum EntityType
{
	ENTITY_TYPE_NONE = 0,

	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_SKELETON,

	ENTITY_TYPE_LAST
};

struct HitParams
{
	int damage = 1;
	float damageMultiplier = 1;
};

struct Entity;
typedef void(*EntityHitCallback_t)(Entity* entity, HitParams hit, Entity* by);

struct Entity
{
	EntityType type;
	bool removed;

	EntityHitCallback_t hitCallback;
};


void InitEntity(Entity* entity, EntityType type);
