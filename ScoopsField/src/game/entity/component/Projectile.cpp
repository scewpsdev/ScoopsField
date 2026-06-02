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
	if (projectile->trail)
	{
		projectile->trail->destroyOnCollapse = true;
	}
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

						if (projectile->trail)
							projectile->trail->destroyOnCollapse = true;
					}
					else
					{
						projectile->removed = true;
					}
					return;
				}
				else
				{
					HitParams hit = {};
					hit.position = hits[i].position;
					HitEntity((Entity*)body->userPtr, &hit, (Entity*)projectile);

					projectile->removed = true;
				}
			}
		}

		if (!projectile->stuck)
		{
			projectile->position = nextPosition;

			if (projectile->rotateForwards)
			{
				projectile->rotation = quat::LookAt(projectile->velocity, vec3::Up) * quat::FromAxisAngle(vec3::AxisZ, projectile->roll);
			}

			if (projectile->offset.lengthSquared() > 0)
				projectile->offset = mix(projectile->offset, vec3::Zero, 5 * deltaTime);

			if (projectile->trail)
			{
				projectile->trail->position = projectile->position + projectile->offset;
			}

			if (projectile->particles)
			{
				projectile->particles->position = projectile->position;
				projectile->particles->rotation = projectile->rotation;
				projectile->particles->scale = projectile->scale;
			}
		}
	}
}

void RenderProjectile(Projectile* projectile)
{
	RenderModel(&game->renderer, projectile->model, projectile->shader, nullptr, mat4::Translate(projectile->offset) * ModelMatrix((Entity*)projectile));

	if (projectile->hasLight)
		RenderLight(&game->renderer, projectile->position, projectile->lightColor);
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

	projectile->trail = (Trail*)CreateEntity();
	InitTrail(projectile->trail, position + projectile->offset, 16);
	projectile->trail->color = vec4(0.5f, 0.5f, 0.5f, 1);
	projectile->trail->texture = GetTexture("textures/effect/trail.png");
	projectile->trail->fadeWidth = true;
	projectile->trail->fadeAlpha = true;
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

	projectile->trail = (Trail*)CreateEntity();
	InitTrail(projectile->trail, position + projectile->offset, 8);
	projectile->trail->width = 0.1f;
	projectile->trail->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 50, 1);
	projectile->trail->texture = GetTexture("textures/effect/trail.png");
	projectile->trail->fadeWidth = true;
	//projectile->trail->fadeAlpha = true;

	projectile->hasLight = true;
	projectile->lightColor = SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 20;

	projectile->particles = (ParticleEffect*)CreateEntity();
	InitParticleEffect(projectile->particles, position, true);
	projectile->particles->startVelocity = vec3(0, 0.5f, 0);
	projectile->particles->gravity = vec3(0, -1, 0);
	projectile->particles->minSize = 0.01f;
	projectile->particles->maxSize = 0.02f,
	projectile->particles->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 5, 1);
	projectile->particles->texture = GetTexture("textures/effect/spark.png");
	projectile->particles->textureSampler = TEXTURE_SAMPLER_LINEAR_CLAMPED;
}
