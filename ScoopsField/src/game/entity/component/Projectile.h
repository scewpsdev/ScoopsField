#pragma once

#include "model/Model.h"

#include "physics/RigidBody.h"


struct Entity;

struct Projectile
{
	vec3 velocity;
	float gravity;
	float drag;
	bool rotateAlongVelocity;
	vec3 hitboxOffset;
	bool stickToObjects;

	vec3 offset;
	bool stuck;
};


void InitProjectile(Entity* entity, vec3 position, vec3 direction, mat4 startTransform, float speed);
void DestroyProjectile(Projectile* projectile, Entity* entity);

bool InteractProjectile(Projectile* projectile, Entity* entity, Entity* by);

void UpdateProjectile(Projectile* projectile, Entity* entity);
void RenderProjectile(Projectile* projectile, Entity* entity);

void InitArrow(Entity* entity, vec3 position, vec3 direction, mat4 startTransform);
void InitMagicProjectile(Entity* entity, vec3 position, vec3 direction, mat4 startTransform);
