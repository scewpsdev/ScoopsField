#pragma once

#include "EntityBase.h"

#include "utils/Optional.h"

#include "game/player/Player.h"
#include "component/Creature.h"
#include "component/ItemEntity.h"
#include "component/RestingSpot.h"
#include "component/Projectile.h"
#include "component/Trail.h"
#include "game/particle/ParticleSystem.h"
#include "component/Ragdoll.h"
#include "component/Sconce.h"


enum EntityPhysicsFilter
{
	ENTITY_FILTER_DEFAULT = 1 << 0,
	ENTITY_FILTER_ENEMY = 1 << 1,
	ENTITY_FILTER_ENEMY_HITBOX = 1 << 2,
	ENTITY_FILTER_PLAYER = 1 << 3,
	ENTITY_FILTER_ITEM = 1 << 4,
	ENTITY_FILTER_INTERACTABLE = 1 << 5,
	ENTITY_FILTER_RAGDOLL = 1 << 6,
};

struct HitParams
{
	int damage = 10;
	float damageMultiplier = 1;
	vec3 position;
	RigidBody* body;
	vec3 impulse;

	bool wasHeadshot;
	bool wasBlocked;
	bool wasParried;
};

struct Entity
{
	union {
		struct {
			EntityType type;
			bool removed;

#define MAX_DESTROY_CALLBACKS 16
			Entity* destroyCallbacks[MAX_DESTROY_CALLBACKS];
			int numDestroyCallbacks;

			vec3 position;
			quat rotation;
			vec3 scale;

			Model* model;
			GraphicsPipeline* shader;
		};

		Player player;
		Creature creature;
		ItemEntity item;
		RestingSpot restingSpot;
		Projectile projectile;
		Trail trail;
		ParticleEffect particles;
		Ragdoll ragdoll;
		Sconce sconce;
	};
};


void InitEntity(Entity* entity, EntityType type);
void DestroyEntity(Entity* entity);
void AddDestroyCallback(Entity* entity, Entity* callbackEntity);

bool HitEntity(Entity* entity, HitParams* hit, Entity* by);
bool InteractEntity(Entity* entity, Entity* by);
void OnEntityDestroyed(Entity* entity, Entity* destroyed);

void UpdateEntity(Entity* entity);
void RenderEntity(Entity* entity);

inline mat4 ModelMatrix(Entity* entity)
{
	return mat4::Translate(entity->position) * mat4::Rotate(entity->rotation) * mat4::Scale(entity->scale);
}
