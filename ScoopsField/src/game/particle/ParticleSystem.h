#pragma once


struct ParticleEffect
{
	vec3* positions;
	vec2* sizes;
	vec4* colors;
	vec3* velocities;
	float* deathTimes;
	int maxParticles;
	int numParticles;

	float minLifetime, maxLifetime;
	float spawnRate;
	float spawnRemainder;
	float minSize, maxSize;
	vec3 startPosition;
	vec3 startVelocity;
	vec3 gravity;

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

	ParticleEffect effect;
};


void InitParticleInstanceBufferLayouts(VertexBufferLayout* instanceLayouts);
void InitParticleSystem(ParticleSystem* particles);
