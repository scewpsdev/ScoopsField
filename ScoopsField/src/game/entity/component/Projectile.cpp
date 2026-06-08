#include "Projectile.h"

#include "Resource.h"

#include "game/player/action/Action.h"


void InitProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, float speed, int damage, Entity* shooter)
{
	InitEntity((Entity*)projectile, ENTITY_TYPE_PROJECTILE);
	projectile->position = position;
	projectile->rotation = startTransform.rotation();
	projectile->scale = vec3::One;

	projectile->velocity = direction * speed;
	projectile->offset = startTransform.translation() - position;

	projectile->damage = damage;

	projectile->shooter = shooter;
}

void DestroyProjectile(Projectile* projectile, Entity* entity)
{
	if (projectile->trail)
	{
		projectile->trail->destroyOnCollapse = true;
	}
	if (projectile->particles)
	{
		projectile->particles->destroyOnFinish = true;
		StopParticleEffect(projectile->particles);
	}
}

void UpdateProjectile(Projectile* projectile)
{
	if (!projectile->stuck)
	{
		projectile->velocity.y += 0.5f * projectile->gravity * deltaTime;
		vec3 dragForce = projectile->velocity * projectile->velocity.length() * projectile->drag;
		projectile->velocity += -dragForce * deltaTime;
		vec3 nextPosition = projectile->position + projectile->velocity * deltaTime;
		projectile->velocity.y += 0.5f * projectile->gravity * deltaTime;

		projectile->roll += projectile->rotationSpeed * deltaTime;

		vec3 d = nextPosition - projectile->position;
		float l = d.length();
		if (l)
		{
			d /= l;

			PhysicsHit hits[16];
			int numHits = Raycast(projectile->position + projectile->rotation * projectile->hitboxOffset, d, l, hits, 16, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY_HITBOX);
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
					hit.damage = projectile->damage;
					hit.position = hits[i].position;
					hit.body = body;
					hit.impulse = projectile->velocity * 0.005f * 40.0f / 30.0f * projectile->damage / 200.0f;
					if (HitEntity((Entity*)body->userPtr, &hit, (Entity*)projectile))
					{
						if (projectile->hitSound)
							PlaySound(projectile->hitSound, projectile->position, 1);

						if (projectile->shooter && projectile->shooter->type == ENTITY_TYPE_PLAYER)
							OnProjectileHit(&projectile->shooter->player, hit.wasHeadshot);

						projectile->removed = true;
					}
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
	if (projectile->model)
		RenderModel(&game->renderer, projectile->model, projectile->shader, nullptr, mat4::Translate(projectile->offset) * ModelMatrix((Entity*)projectile));

	if (projectile->hasLight)
		RenderLight(&game->renderer, projectile->position, projectile->lightColor);
}


void InitArrow(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, Entity* shooter)
{
	float speed = 40;
	int damage = 40;
	InitProjectile(projectile, position, direction, startTransform, speed, damage, shooter);

	projectile->model = GetModel("items/arrow/arrow.glb");
	projectile->hitSound = &game->hitArrowSound;

	projectile->gravity = -10;
	projectile->rotateForwards = true;
	projectile->hitboxOffset = vec3(0, 0, -0.7f);
	projectile->stickToObjects = true;

	projectile->trail = (Trail*)CreateEntity();
	InitTrail(projectile->trail, position + projectile->offset, false);
	projectile->trail->color = vec4(0.5f, 0.5f, 0.5f, 1);
	projectile->trail->texture = GetTexture("textures/effect/trail.png");
	projectile->trail->fadeWidth = true;
	projectile->trail->fadeAlpha = true;
}

void InitMagicProjectile(Projectile* projectile, vec3 position, vec3 direction, mat4 startTransform, Entity* shooter)
{
	float speed = 30;
	int damage = 50;
	InitProjectile(projectile, position, direction, startTransform, speed, damage, shooter);

	//projectile->model = GetModel("entities/projectile/magic_projectile/magic_projectile.glb");
	//projectile->shader = game->magicProjectileShader;

	projectile->gravity = -4.0f;
	projectile->rotationSpeed = 5 * PI;
	projectile->rotateForwards = true;
	projectile->drag = 0.1f;

	projectile->trail = (Trail*)CreateEntity();
	InitTrail(projectile->trail, position + projectile->offset, true);
	projectile->trail->width = 0.1f;
	projectile->trail->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 5, 0.3f);
	projectile->trail->texture = GetTexture("textures/effect/trail.png");
	projectile->trail->fadeWidth = true;
	projectile->trail->fadeAlpha = true;

	projectile->hasLight = true;
	projectile->lightColor = SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 15;

	projectile->particles = (ParticleEffect*)CreateEntity();
	LoadParticleEffect(projectile->particles, "effects/projectile/magic_projectile.rfs", position, projectile->rotation);

	/*
	InitParticleEffect(projectile->particles, position);

	ParticleEmitter* sparks = AddEmitter(projectile->particles, true, 1000, 1, 2);
	sparks->startVelocity = vec3(0, 1, 0);
	sparks->randomDirection = 0.3f;
	sparks->randomVelocity = 1;
	sparks->gravity = vec3(0, -3, 0);
	sparks->size = 0.05f;
	sparks->endSize = 0.05f;
	sparks->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 10, 1);
	sparks->endColor = sparks->color * vec4(1, 1, 1, 0);
	sparks->texture = GetTexture("textures/effect/spark.png");
	sparks->textureSampler = TEXTURE_SAMPLER_LINEAR_CLAMPED;

	ParticleEmitter* rings = AddEmitter(projectile->particles, true, 8, 0.4f, 0.4f);
	rings->size = 0.1f;
	rings->endSize = 1.0f;
	rings->color = vec4(SRGBToLinear(ARGBToVector(0xFF95C8FF)).xyz * 2, 1);
	rings->endColor = rings->color * vec4(1, 1, 1, 0);
	rings->texture = GetTexture("textures/effect/ring.png");
	rings->textureSampler = TEXTURE_SAMPLER_LINEAR_CLAMPED;
	*/
}
