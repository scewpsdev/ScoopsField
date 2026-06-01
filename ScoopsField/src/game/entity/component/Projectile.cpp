#include "Projectile.h"

#include "Resource.h"

#include "game/player/action/Action.h"


void InitProjectile(Entity* entity, vec3 position, vec3 direction, mat4 startTransform, float speed)
{
	InitEntity(entity, ENTITY_TYPE_PROJECTILE);
	entity->position = position;
	entity->rotation = startTransform.rotation();
	entity->scale = vec3::One;

	entity->projectile.velocity = direction * speed;
	entity->projectile.offset = startTransform.translation() - position;
}

void DestroyProjectile(Projectile* projectile, Entity* entity)
{
}

bool InteractProjectile(Projectile* projectile, Entity* entity, Entity* by)
{
	return false;
}

void UpdateProjectile(Projectile* projectile, Entity* entity)
{
	if (!projectile->stuck)
	{
		entity->projectile.velocity.y += 0.5f * entity->projectile.gravity * deltaTime;
		entity->projectile.velocity += entity->projectile.drag * entity->projectile.velocity.lengthSquared() * -entity->projectile.velocity.normalized() / 2;
		vec3 nextPosition = entity->position + entity->projectile.velocity * deltaTime;
		entity->projectile.velocity.y += 0.5f * entity->projectile.gravity * deltaTime;

		entity->projectile.rotation += entity->projectile.rotationSpeed * deltaTime;

		vec3 d = nextPosition - entity->position;
		float l = d.length();
		if (l)
		{
			d /= l;

			PhysicsHit hits[16];
			int numHits = Raycast(entity->position + entity->rotation * projectile->hitboxOffset, d, l, hits, 16, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY);
			for (int i = 0; i < numHits; i++)
			{
				RigidBody* body = hits[i].body;
				if (body->type == RIGID_BODY_STATIC)
				{
					if (projectile->stickToObjects)
					{
						projectile->stuck = true;
						entity->position += d * hits[i].distance;
					}
					else
					{
						if (projectile->hasTrail)
							projectile->stuck = true;
						else
							entity->removed = true;
					}
					return;
				}
				else
				{
					HitParams hit = {};
					hit.position = hits[i].position;
					HitEntity((Entity*)body->userPtr, &hit, entity);

					if (projectile->hasTrail)
						projectile->stuck = true;
					else
						entity->removed = true;
				}
			}
		}

		if (!projectile->stuck)
		{
			entity->position = nextPosition;

			if (entity->projectile.rotateForwards)
			{
				entity->rotation = quat::LookAt(entity->projectile.velocity, quat::FromAxisAngle(vec3::AxisZ, entity->projectile.rotation) * vec3::Up);
			}

			if (entity->projectile.offset.lengthSquared() > 0)
				entity->projectile.offset = mix(entity->projectile.offset, vec3::Zero, 5 * deltaTime);
		}
	}

	if (projectile->hasTrail)
	{
		bool isDead = !projectile->stickToObjects && projectile->stuck;

		UpdateTrail(&projectile->trail, entity->position + entity->projectile.offset, cmdBuffer);

		if (isDead && projectile->trail.nodes[projectile->trail.numNodes - 1].position == entity->position + entity->projectile.offset)
			entity->removed = true;
	}
}

void RenderProjectile(Projectile* projectile, Entity* entity)
{
	bool isDead = projectile->hasTrail && !projectile->stickToObjects && projectile->stuck;

	if (!isDead)
	{
		RenderModel(&game->renderer, entity->model, entity->shader, nullptr, mat4::Translate(projectile->offset) * ModelMatrix(entity));

		if (projectile->hasLight)
			RenderLight(&game->renderer, entity->position, projectile->lightColor);
	}

	if (projectile->hasTrail)
		RenderTrail(&projectile->trail);
}


void InitArrow(Entity* entity, vec3 position, vec3 direction, mat4 startTransform)
{
	float speed = 40;
	InitProjectile(entity, position, direction, startTransform, speed);

	entity->model = GetModel("items/arrow/arrow.glb");

	entity->projectile.gravity = -10;
	entity->projectile.rotateForwards = true;
	entity->projectile.hitboxOffset = vec3(0, 0, -0.7f);
	entity->projectile.stickToObjects = true;

	entity->projectile.hasTrail = true;
	InitTrail(&entity->projectile.trail, position + entity->projectile.offset, 16);
	entity->projectile.trail.color = vec4(0.5f, 0.5f, 0.5f, 1);
	entity->projectile.trail.fadeWidth = true;
	entity->projectile.trail.fadeAlpha = true;
}

void InitMagicProjectile(Entity* entity, vec3 position, vec3 direction, mat4 startTransform)
{
	float speed = 20;
	InitProjectile(entity, position, direction, startTransform, speed);

	entity->model = GetModel("entities/projectile/magic_projectile/magic_projectile.glb");
	entity->shader = game->magicProjectileShader;

	entity->projectile.gravity = -1.5f;
	entity->projectile.rotationSpeed = 5 * PI;
	entity->projectile.rotateForwards = true;
	entity->projectile.drag = 0.001f;

	entity->projectile.hasTrail = true;
	InitTrail(&entity->projectile.trail, position + entity->projectile.offset, 8);
	entity->projectile.trail.width = 0.1f;
	entity->projectile.trail.color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 50, 1);
	entity->projectile.trail.material = &game->trailMaterial;
	entity->projectile.trail.fadeWidth = true;
	//entity->projectile.trail.fadeAlpha = true;

	entity->projectile.hasLight = true;
	entity->projectile.lightColor = SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 5;
}
