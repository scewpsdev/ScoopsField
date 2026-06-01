#include "Projectile.h"

#include "Resource.h"

#include "game/player/action/Action.h"


void InitProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, float speed)
{
	InitEntity((Entity*)projectile, ENTITY_TYPE_PROJECTILE);
	projectile->position = position;
	projectile->rotation = startTransform.rotation();
	projectile->scale = vec3::One;

	projectile->velocity = direction * speed;
	projectile->offset = startTransform.translation() - position;
}

void DestroyProjectile(Projectile* projectile, Entity* entity)
{
	if (projectile->particles)
	{
		projectile->particles->spawnRate = 0;
		projectile->particles->destroyOnFinish = true;
	}
}

void UpdateProjectile(Projectile* projectile)
{
	if (!projectile->stuck)
	{
		projectile->velocity.y += 0.5f * projectile->gravity * deltaTime;
		projectile->velocity += projectile->drag * projectile->velocity.lengthSquared() * -projectile->velocity.normalized() / 2;
		vec3 nextPosition = projectile->position + projectile->velocity * deltaTime;
		projectile->velocity.y += 0.5f * projectile->gravity * deltaTime;

		projectile->roll += projectile->rotationSpeed * deltaTime;

		vec3 d = nextPosition - projectile->position;
		float l = d.length();
		if (l)
		{
			d /= l;

			PhysicsHit hits[16];
			int numHits = Raycast(projectile->position + projectile->rotation * projectile->hitboxOffset, d, l, hits, 16, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY);
			for (int i = 0; i < numHits; i++)
			{
				RigidBody* body = hits[i].body;
				if (body->type == RIGID_BODY_STATIC)
				{
					if (projectile->stickToObjects)
					{
						projectile->stuck = true;
						projectile->position += d * hits[i].distance;
					}
					else
					{
						if (projectile->hasTrail)
							projectile->stuck = true;
						else
							projectile->removed = true;
					}
					return;
				}
				else
				{
					HitParams hit = {};
					hit.position = hits[i].position;
					HitEntity((Entity*)body->userPtr, &hit, (Entity*)projectile);

					if (projectile->hasTrail)
						projectile->stuck = true;
					else
						projectile->removed = true;
				}
			}
		}

		if (!projectile->stuck)
		{
			projectile->position = nextPosition;

			if (projectile->rotateForwards)
			{
				projectile->rotation = quat::LookAt(projectile->velocity, quat::FromAxisAngle(vec3::AxisZ, projectile->roll) * vec3::Up);
			}

			if (projectile->offset.lengthSquared() > 0)
				projectile->offset = mix(projectile->offset, vec3::Zero, 5 * deltaTime);

			if (projectile->particles)
			{
				projectile->particles->position = projectile->position;
				projectile->particles->rotation = projectile->rotation;
				projectile->particles->scale = projectile->scale;
			}
		}
	}

	if (projectile->hasTrail)
	{
		bool isDead = !projectile->stickToObjects && projectile->stuck;

		UpdateTrail(&projectile->trail, projectile->position + projectile->offset, cmdBuffer);

		if (isDead && projectile->trail.nodes[projectile->trail.numNodes - 1].position == projectile->position + projectile->offset)
			projectile->removed = true;
	}
}

void RenderProjectile(Projectile* projectile)
{
	bool isDead = projectile->hasTrail && !projectile->stickToObjects && projectile->stuck;

	if (!isDead)
	{
		RenderModel(&game->renderer, projectile->model, projectile->shader, nullptr, mat4::Translate(projectile->offset) * ModelMatrix((Entity*)projectile));

		if (projectile->hasLight)
			RenderLight(&game->renderer, projectile->position, projectile->lightColor);
	}

	if (projectile->hasTrail)
		RenderTrail(&projectile->trail);
}


void InitArrow(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform)
{
	float speed = 40;
	InitProjectile(projectile, position, direction, startTransform, speed);

	projectile->model = GetModel("items/arrow/arrow.glb");

	projectile->gravity = -10;
	projectile->rotateForwards = true;
	projectile->hitboxOffset = vec3(0, 0, -0.7f);
	projectile->stickToObjects = true;

	projectile->hasTrail = true;
	InitTrail(&projectile->trail, position + projectile->offset, 16);
	projectile->trail.color = vec4(0.5f, 0.5f, 0.5f, 1);
	projectile->trail.fadeWidth = true;
	projectile->trail.fadeAlpha = true;
}

void InitMagicProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform)
{
	float speed = 20;
	InitProjectile(projectile, position, direction, startTransform, speed);

	projectile->model = GetModel("entities/projectile/magic_projectile/magic_projectile.glb");
	projectile->shader = game->magicProjectileShader;

	projectile->gravity = -1.5f;
	projectile->rotationSpeed = 5 * PI;
	projectile->rotateForwards = true;
	projectile->drag = 0.001f;

	projectile->hasTrail = true;
	InitTrail(&projectile->trail, position + projectile->offset, 8);
	projectile->trail.width = 0.1f;
	projectile->trail.color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 50, 1);
	projectile->trail.material = &game->trailMaterial;
	projectile->trail.fadeWidth = true;
	//projectile->trail.fadeAlpha = true;

	projectile->hasLight = true;
	projectile->lightColor = SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 15;

	projectile->particles = (ParticleEffect*)CreateEntity();
	InitParticleEffect(projectile->particles);
	projectile->particles->startPosition = vec3(0, 0.1f, 0);
	projectile->particles->startVelocity = vec3(0, 5, 0);
	projectile->particles->gravity = vec3(0, -5, 0);
	projectile->particles->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 10, 1);
}
