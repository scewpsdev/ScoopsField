#pragma once

#include "model/Model.h"

#include "physics/RigidBody.h"


struct Entity;

struct Projectile
{
};


void InitProjectile(Entity* entity, vec3 position, quat rotation);
void DestroyProjectile(Projectile* projectile, Entity* entity);

bool InteractProjectile(Projectile* projectile, Entity* entity, Entity* by);

void UpdateProjectile(Projectile* projectile, Entity* entity);
void RenderProjectile(Projectile* projectile, Entity* entity);
