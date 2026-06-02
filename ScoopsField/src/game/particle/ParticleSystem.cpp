#include "ParticleSystem.h"


void InitParticleInstanceBufferLayouts(VertexBufferLayout* instanceLayouts)
{
	instanceLayouts[0].numAttributes = 1;
	instanceLayouts[0].attributes[0].location = 1;
	instanceLayouts[0].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	instanceLayouts[0].perInstance = true;

	instanceLayouts[1].numAttributes = 1;
	instanceLayouts[1].attributes[0].location = 2;
	instanceLayouts[1].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	instanceLayouts[1].perInstance = true;

	instanceLayouts[2].numAttributes = 1;
	instanceLayouts[2].attributes[0].location = 3;
	instanceLayouts[2].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayouts[2].perInstance = true;
}

void InitParticleEffect(ParticleEffect* effect, vec3 position, bool additive)
{
	game->particles.effects.add(effect);

	InitEntity((Entity*)effect, ENTITY_TYPE_PARTICLE_EFFECT);

	effect->position = position;
	effect->shader = additive ? game->particleAdditiveShader : game->particleShader;

	effect->minLifetime = 1;
	effect->maxLifetime = 2;
	effect->spawnRate = 1000;
	effect->minSize = 0.02f;
	effect->maxSize = 0.1f;
	effect->startPosition = vec3(0);
	effect->startVelocity = vec3(0);
	effect->gravity = vec3(0);
	effect->color = vec4(1);

	int maxParticles = (int)SDL_ceilf(effect->maxLifetime * effect->spawnRate);

	effect->maxParticles = maxParticles;
	effect->numParticles = 0;

	effect->positions = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	effect->sizes = (vec2*)SDL_malloc(maxParticles * sizeof(vec2));
	effect->colors = (vec4*)SDL_malloc(maxParticles * sizeof(vec4));
	effect->velocities = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	effect->deathTimes = (float*)SDL_malloc(maxParticles * sizeof(float));

	VertexBufferLayout instanceLayouts[3];
	InitParticleInstanceBufferLayouts(instanceLayouts);

	effect->positionBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[0], SDL_GPU_BUFFERUSAGE_VERTEX);
	effect->sizeBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[1], SDL_GPU_BUFFERUSAGE_VERTEX);
	effect->colorBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[2], SDL_GPU_BUFFERUSAGE_VERTEX);

	effect->positionTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec3), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	effect->sizeTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec2), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	effect->colorTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec4), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
}

void DestroyParticleEffect(ParticleEffect* effect)
{
	game->particles.effects.remove(effect);

	DestroyVertexBuffer(effect->positionBuffer);
	DestroyVertexBuffer(effect->sizeBuffer);
	DestroyVertexBuffer(effect->colorBuffer);

	DestroyTransferBuffer(effect->positionTransferBuffer);
	DestroyTransferBuffer(effect->sizeTransferBuffer);
	DestroyTransferBuffer(effect->colorTransferBuffer);

	SDL_free(effect->positions);
	SDL_free(effect->sizes);
	SDL_free(effect->colors);
	SDL_free(effect->velocities);
	SDL_free(effect->deathTimes);
}

static void SpawnParticle(ParticleEffect* effect)
{
	SDL_assert(effect->numParticles < effect->maxParticles);

	int particleID = effect->numParticles++;

	mat4 transform = ModelMatrix((Entity*)effect);

	effect->positions[particleID] = (transform * vec4(effect->startPosition, 1)).xyz;

	effect->sizes[particleID] = vec2(mix(effect->minSize, effect->maxSize, game->random.nextFloat()));

	effect->colors[particleID] = effect->color;

	effect->velocities[particleID] = (transform * vec4(effect->startVelocity + game->random.nextVector3(-1, 1) * 0.5f, 0)).xyz;

	float lifetime = mix(effect->minLifetime, effect->maxLifetime, game->random.nextFloat());
	effect->deathTimes[particleID] = gameTime + lifetime;
}

static void KillParticle(ParticleEffect* effect, int id)
{
	int lastParticle = --effect->numParticles;

	if (id != lastParticle)
	{
		effect->positions[id] = effect->positions[lastParticle];
		effect->sizes[id] = effect->sizes[lastParticle];
		effect->colors[id] = effect->colors[lastParticle];
		effect->velocities[id] = effect->velocities[lastParticle];
		effect->deathTimes[id] = effect->deathTimes[lastParticle];
	}
}

static void UpdateParticleEffect(ParticleEffect* effect)
{
	for (int i = 0; i < effect->numParticles; i++)
	{
		if (gameTime >= effect->deathTimes[i])
			KillParticle(effect, i--);
	}

	for (int i = 0; i < effect->numParticles; i++)
		effect->velocities[i] += 0.5f * effect->gravity * deltaTime;

	for (int i = 0; i < effect->numParticles; i++)
		effect->positions[i] += effect->velocities[i] * deltaTime;

	for (int i = 0; i < effect->numParticles; i++)
		effect->velocities[i] += 0.5f * effect->gravity * deltaTime;

	float numSpawnsF = effect->spawnRate * deltaTime + effect->spawnRemainder;
	int numSpawns = (int)numSpawnsF;
	effect->spawnRemainder = numSpawnsF - numSpawns;

	for (int i = 0; i < numSpawns; i++)
		SpawnParticle(effect);


	if (effect->numParticles)
	{
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		void* mapped = MapTransferBuffer(effect->positionTransferBuffer);
		SDL_memcpy(mapped, effect->positions, effect->numParticles * sizeof(vec3));
		UnmapTransferBuffer(effect->positionTransferBuffer);

		mapped = MapTransferBuffer(effect->sizeTransferBuffer);
		SDL_memcpy(mapped, effect->sizes, effect->numParticles * sizeof(vec2));
		UnmapTransferBuffer(effect->sizeTransferBuffer);

		mapped = MapTransferBuffer(effect->colorTransferBuffer);
		SDL_memcpy(mapped, effect->colors, effect->numParticles * sizeof(vec4));
		UnmapTransferBuffer(effect->colorTransferBuffer);

		UpdateVertexBuffer(effect->positionBuffer, 0, effect->numParticles * sizeof(vec3), effect->positionTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(effect->sizeBuffer, 0, effect->numParticles * sizeof(vec2), effect->sizeTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(effect->colorBuffer, 0, effect->numParticles * sizeof(vec4), effect->colorTransferBuffer->buffer, copyPass);

		SDL_EndGPUCopyPass(copyPass);
	}

	if (effect->destroyOnFinish && effect->spawnRate == 0 && effect->numParticles == 0)
		effect->removed = true;
}

static void RenderParticleEffect(ParticleSystem* particles, ParticleEffect* effect)
{
	VertexBuffer* buffers[4];
	buffers[0] = particles->quad;
	buffers[1] = effect->positionBuffer;
	buffers[2] = effect->sizeBuffer;
	buffers[3] = effect->colorBuffer;

	vec4 params = vec4(effect->texture ? 1.0f : 0.0f, 0, 0, 0);

	RenderMesh(&game->renderer, buffers, 4, nullptr, 4, effect->numParticles, {}, {}, &params, sizeof(params), &effect->texture, &effect->textureSampler, 1, effect->shader, mat4::Identity);
}

const vec2 quadVertices[] = {
	vec2(0.5f, -0.5f),
	vec2(0.5f, 0.5f),
	vec2(-0.5f, -0.5f),
	vec2(-0.5f, 0.5f),
};

void InitParticleSystem(ParticleSystem* particles)
{
	VertexBufferLayout quadLayout = {};
	quadLayout.numAttributes = 1;
	quadLayout.attributes[0].location = 0;
	quadLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

	particles->quad = CreateVertexBuffer(4, &quadLayout, SDL_GPU_BUFFERUSAGE_VERTEX);
	UpdateVertexBuffer(particles->quad, 0, (const uint8_t*)quadVertices, sizeof(quadVertices), cmdBuffer);
}

void UpdateParticleSystem(ParticleSystem* particles)
{
	for (int i = 0; i < particles->effects.size; i++)
	{
		UpdateParticleEffect(particles->effects[i]);
	}
}

void RenderParticleSystem(ParticleSystem* particles)
{
	for (int i = 0; i < particles->effects.size; i++)
	{
		RenderParticleEffect(particles, particles->effects[i]);
	}
}
