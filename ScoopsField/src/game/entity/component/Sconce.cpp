#include "RestingSpot.h"

#include "Application.h"

#include "game/player/action/Action.h"
#include "game/particle/ParticleSystem.h"

#include "renderer/Renderer.h"


void InitSconce(Sconce* sconce, vec3 position)
{
	InitEntity((Entity*)sconce, ENTITY_TYPE_SCONCE);
	sconce->position = position;
	sconce->model = GetModel("entities/object/sconce/sconce.glb");

	InitRigidBody(&sconce->body, RIGID_BODY_STATIC, sconce->position, quat::Identity, sconce);
	AddCapsuleCollider(&sconce->body, 0.25f, 1.2f, vec3(0, 0.6f, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_INTERACTABLE, 0, false);

	sconce->particles = (ParticleEffect*)CreateEntity();
	LoadParticleEffect(sconce->particles, "effects/object/sconce.rfs", position, quat::Identity);

	sconce->burning = false;

	SDL_assert(sconce->particles->numEmitters <= 8);
	for (int i = 0; i < sconce->particles->numEmitters; i++)
	{
		sconce->particlesSpawnRate[i] = sconce->particles->emitters[i].spawnRate;
		sconce->particles->emitters[i].spawnRate = 0;
	}

	InteractSconce(sconce, nullptr);
}

void DestroySconce(Sconce* sconce)
{
	DestroyRigidBody(&sconce->body);

	sconce->particles->removed = true;
}

bool InteractSconce(Sconce* sconce, Entity* by)
{
	sconce->burning = !sconce->burning;
	for (int i = 0; i < sconce->particles->numEmitters; i++)
	{
		sconce->particles->emitters[i].spawnRate = sconce->burning ? sconce->particlesSpawnRate[i] : 0;
	}

	if (sconce->burning)
		PlaySound(&game->fireSound, sconce->position + vec3(0, 1.5f, 0));

	return true;
}

void UpdateSconce(Sconce* sconce)
{
}

void RenderSconce(Sconce* sconce)
{
	RenderModel(&game->renderer, sconce->model, nullptr, ModelMatrix((Entity*)sconce));

	if (sconce->particles->emitters[0].numParticles)
	{
		float brightness = sconce->particles->emitters[0].numParticles / (float)sconce->particles->emitters[0].maxParticles;
		//RenderLight(&game->renderer, sconce->position + vec3(0, 1.2f, 0), SRGBToLinear(ARGBToVector(0xFFFF6400)).rgb * brightness * 5);
	}
}
