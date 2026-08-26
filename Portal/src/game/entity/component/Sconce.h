#pragma once

#include "game/entity/EntityBase.h"

#include "model/Model.h"

#include "physics/RigidBody.h"


struct Entity;
struct ParticleEffect;

struct Sconce : EntityBase
{
	RigidBody body;

	ParticleEffect* particles;
	float particlesSpawnRate[8];

	bool burning;
};


void InitSconce(Sconce* sconce, vec3 position);
void DestroySconce(Sconce* sconce);

bool InteractSconce(Sconce* sconce, Entity* by);

void UpdateSconce(Sconce* sconce);
void RenderSconce(Sconce* sconce);
