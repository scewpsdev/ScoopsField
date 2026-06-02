#pragma once

#include "game/entity/EntityBase.h"


struct ParticleEmitter
{
	vec3* positions;
	vec2* sizes;
	vec4* colors;
	vec3* velocities;
	float* birthTimes;
	float* deathTimes;
	int maxParticles;
	int numParticles;

	float spawnRate;
	float spawnRemainder;

	float minLifetime, maxLifetime;
	float startSize, endSize;
	vec3 startPosition;
	vec3 startVelocity;
	vec3 randomVelocity;
	vec3 gravity;
	vec4 startColor;
	vec4 endColor;
	Texture* texture;
	TextureSampler textureSampler;

	GraphicsPipeline* shader;

	VertexBuffer* positionBuffer;
	VertexBuffer* sizeBuffer;
	VertexBuffer* colorBuffer;

	TransferBuffer* positionTransferBuffer;
	TransferBuffer* sizeTransferBuffer;
	TransferBuffer* colorTransferBuffer;
};

struct ParticleEffect : EntityBase
{
#define MAX_EMITTERS 8
	ParticleEmitter emitters[MAX_EMITTERS];
	int numEmitters;

	bool destroyOnFinish;
};

struct ParticleSystem
{
	VertexBuffer* quad;

#define MAX_PARTICLE_EFFECTS 64
	List<ParticleEffect*, MAX_PARTICLE_EFFECTS> effects;
};


void InitParticleEffect(ParticleEffect* effect, vec3 position);
void DestroyParticleEffect(ParticleEffect* effect);
ParticleEmitter* AddEmitter(ParticleEffect* effect, bool additive, float spawnRate, float minLifetime, float maxLifetime);

void InitParticleInstanceBufferLayouts(VertexBufferLayout* instanceLayouts);
void InitParticleSystem(ParticleSystem* particles);
