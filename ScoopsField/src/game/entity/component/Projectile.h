#pragma once

#include "model/Model.h"

#include "physics/RigidBody.h"

#include "game/entity/EntityBase.h"	


struct Trail;
struct ParticleEffect;

struct Projectile : EntityBase
{
	vec3 velocity;
	float gravity;
	float drag;
	float rotationSpeed;
	bool rotateForwards;
	vec3 hitboxOffset;
	bool stickToObjects;

	vec3 offset;
	float roll;
	bool stuck;

	bool hasLight;
	vec3 lightColor;

	Trail* trail;
	ParticleEffect* particles;
};


void InitProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, float speed);
void DestroyProjectile(Projectile* projectile, Entity* entity);

void UpdateProjectile(Projectile* projectile);
void RenderProjectile(Projectile* projectile);

void InitArrow(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform);
void InitMagicProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform);
