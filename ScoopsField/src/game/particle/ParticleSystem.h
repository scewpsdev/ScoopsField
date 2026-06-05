#pragma once

#include "game/entity/EntityBase.h"


enum SpawnShape
{
	SPAWN_SHAPE_POINT,
	SPAWN_SHAPE_SPHERE,
	SPAWN_SHAPE_CIRCLE,
	SPAWN_SHAPE_BOX,
	SPAWN_SHAPE_LINE,
};

struct ParticleEmitter
{
	vec3* positions;
	vec2* sizes;
	float* rotations;
	vec3* velocities;
	vec4* colors;
	float* animations;
	float* birthTimes;
	float* deathTimes;
	int maxParticles;
	int numParticles;

	float spawnRate;
	float spawnRemainder;
	SpawnShape spawnShape;
	float spawnRadius;
	vec3 spawnPoint0, spawnPoint1;
	vec3 spawnSize;

	float minLifetime, maxLifetime;
	float size, endSize;
	vec3 startPosition;

	vec3 gravity;
	float drag;
	vec3 startVelocity;
	float randomDirection;
	bool randomDirectionUniform;
	float randomVelocity;
	float velocityNoise;
	bool inheritVelocity;
	bool inheritCentrifugal;
	bool randomRotation;
	float rotationSpeed;
	float randomRotationSpeed;
	vec4 color;
	vec4 endColor;
	Texture* texture;
	TextureSampler textureSampler;
	ivec2 atlasSize;
	int atlasFrameCount;

	//bool isBurst;
	//float burstTime;
	int burstCount;
	float burstDuration;
	float burstRemainder;
	//float burstDuration;
	//bool hasBursted;

	GraphicsPipeline* shader;

	VertexBuffer* positionBuffer;
	VertexBuffer* sizeBuffer;
	VertexBuffer* rotationBuffer;
	VertexBuffer* colorBuffer;
	VertexBuffer* animationBuffer;

	TransferBuffer* positionTransferBuffer;
	TransferBuffer* sizeTransferBuffer;
	TransferBuffer* rotationTransferBuffer;
	TransferBuffer* colorTransferBuffer;
	TransferBuffer* animationTransferBuffer;
};

struct ParticleEffect : EntityBase
{
#define MAX_EMITTERS 8
	ParticleEmitter emitters[MAX_EMITTERS];
	int numEmitters;

	vec3 lastPosition;
	quat lastRotation;
	float lastUpdate;

	bool destroyOnFinish;
};

struct ParticleSystem
{
	VertexBuffer* quad;

#define MAX_PARTICLE_EFFECTS 64
	List<ParticleEffect*, MAX_PARTICLE_EFFECTS> effects;
};


void InitParticleEffect(ParticleEffect* effect, vec3 position, quat rotation);
void DestroyParticleEffect(ParticleEffect* effect);
ParticleEmitter* AddEmitter(ParticleEffect* effect, bool additive, float spawnRate, float minLifetime, float maxLifetime, int burstCount = 0);

void InitParticleInstanceBufferLayouts(VertexBufferLayout* instanceLayouts);
void InitParticleSystem(ParticleSystem* particles);

void LoadParticleEffect(ParticleEffect* effect, const char* path, vec3 position, quat rotation);
