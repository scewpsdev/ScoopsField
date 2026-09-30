#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"


#define SWITCH_ENTITY() \
ENTITY_TYPE(PLAYER, Player, player)\
ENTITY_TYPE(CREATURE, Creature, creature)\
ENTITY_TYPE(ITEM, ItemEntity, item)\
ENTITY_TYPE(RESTING_SPOT, RestingSpot, restingSpot)\
ENTITY_TYPE(PROJECTILE, Projectile, projectile)\
ENTITY_TYPE(TRAIL, Trail, trail)\
ENTITY_TYPE(PARTICLE_EFFECT, ParticleEffect, particles)\
ENTITY_TYPE(RAGDOLL, Ragdoll, ragdoll)\
ENTITY_TYPE(SCONCE, Sconce, sconce)\
ENTITY_TYPE(ELEVATOR, Elevator, elevator)\
ENTITY_TYPE(TREE, Tree, tree)\
ENTITY_TYPE(WOOD_FLOOR, WoodFloor, woodFloor)\


enum EntityType
{
	ENTITY_TYPE_NONE = 0,

#define ENTITY_TYPE(caps, pascal, camel) ENTITY_TYPE_##caps,
	SWITCH_ENTITY()
#undef ENTITY_TYPE

	ENTITY_TYPE_LAST
};

struct Entity;
struct Model;
struct GraphicsPipeline;
struct RigidBody;

struct EntityBase
{
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
	GraphicsPipeline* shadowShader;
};
