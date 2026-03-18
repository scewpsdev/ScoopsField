#pragma once

#include "model/Model.h"

#include "physics/RigidBody.h"


struct Entity;

struct RestingSpot
{
	RigidBody body;
};


void InitRestingSpot(Entity* entity, vec3 position, quat rotation);
void DestroyRestingSpot(RestingSpot* item, Entity* entity);

bool InteractRestingSpot(RestingSpot* item, Entity* entity, Entity* by);

void UpdateRestingSpot(RestingSpot* item, Entity* entity);
void RenderRestingSpot(RestingSpot* item, Entity* entity);
