#pragma once

#include "game/entity/EntityBase.h"


struct ParticleEffect : EntityBase
{
	vec3* positions;
	vec2* sizes;
	vec4* colors;
	vec3* velocities;
	float* deathTimes;
	int maxParticles;
	int numParticles;

	float spawnRate;
	float spawnRemainder;
	bool destroyOnFinish;

	float minLifetime, maxLifetime;
	float minSize, maxSize;
	vec3 startPosition;
	vec3 startVelocity;
	vec3 gravity;
	vec4 color;

	VertexBuffer* positionBuffer;
	VertexBuffer* sizeBuffer;
	VertexBuffer* colorBuffer;

	TransferBuffer* positionTransferBuffer;
	TransferBuffer* sizeTransferBuffer;
	TransferBuffer* colorTransferBuffer;
};

struct ParticleSystem
{
	VertexBuffer* quad;

#define MAX_PARTICLE_EFFECTS 64
	List<ParticleEffect*, MAX_PARTICLE_EFFECTS> effects;
};


void InitParticleEffect(ParticleEffect* effect);
void DestroyParticleEffect(ParticleEffect* effect);

void InitParticleInstanceBufferLayouts(VertexBufferLayout* instanceLayouts);
void InitParticleSystem(ParticleSystem* particles);
