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
	RigidBody* stuckToBody;
	mat4 stuckLocalTransform;

	bool hasLight;
	vec3 lightColor;

	Trail* trail;
	ParticleEffect* particles;

	Sound* hitSound;
	const char* hitEffect;

	int damage;

	Entity* shooter;
};


void InitProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, float speed, int damage, Entity* shooter);
void DestroyProjectile(Projectile* projectile, Entity* entity);

void OnEntityDestroyed(Projectile* projectile, Entity* destroyed);

void UpdateProjectile(Projectile* projectile);
void RenderProjectile(Projectile* projectile);

void InitArrow(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, Entity* shooter);
void InitMagicProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, Entity* shooter);
