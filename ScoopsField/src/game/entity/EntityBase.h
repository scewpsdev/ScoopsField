#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"


enum EntityType
{
	ENTITY_TYPE_NONE = 0,

	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_CREATURE,
	ENTITY_TYPE_ITEM,
	ENTITY_TYPE_RESTING_SPOT,
	ENTITY_TYPE_PROJECTILE,
	ENTITY_TYPE_TRAIL,
	ENTITY_TYPE_PARTICLE_EFFECT,

	ENTITY_TYPE_LAST
};

struct Model;
struct GraphicsPipeline;

struct EntityBase
{
	EntityType type;
	bool removed;

	vec3 position;
	quat rotation;
	vec3 scale;

	Model* model;
	GraphicsPipeline* shader;
};
