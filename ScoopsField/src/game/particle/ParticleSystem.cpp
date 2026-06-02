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

void InitParticleEmitter(ParticleEmitter* emitter, bool additive, float spawnRate, float minLifetime, float maxLifetime)
{
	emitter->shader = additive ? game->particleAdditiveShader : game->particleShader;

	emitter->minLifetime = minLifetime;
	emitter->maxLifetime = maxLifetime;
	emitter->spawnRate = spawnRate;
	emitter->startSize = 0.1f;
	emitter->endSize = 0.1f;
	emitter->startPosition = vec3(0);
	emitter->startVelocity = vec3(0);
	emitter->gravity = vec3(0);
	emitter->startColor = vec4(1);
	emitter->endColor = vec4(1);

	int maxParticles = (int)SDL_ceilf(emitter->maxLifetime * emitter->spawnRate);

	emitter->maxParticles = maxParticles;
	emitter->numParticles = 0;

	emitter->positions = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	emitter->sizes = (vec2*)SDL_malloc(maxParticles * sizeof(vec2));
	emitter->colors = (vec4*)SDL_malloc(maxParticles * sizeof(vec4));
	emitter->velocities = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	emitter->birthTimes = (float*)SDL_malloc(maxParticles * sizeof(float));
	emitter->deathTimes = (float*)SDL_malloc(maxParticles * sizeof(float));

	VertexBufferLayout instanceLayouts[3];
	InitParticleInstanceBufferLayouts(instanceLayouts);

	emitter->positionBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[0], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->sizeBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[1], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->colorBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[2], SDL_GPU_BUFFERUSAGE_VERTEX);

	emitter->positionTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec3), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->sizeTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec2), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->colorTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec4), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
}

void DestroyParticleEmitter(ParticleEmitter* emitter)
{
	DestroyVertexBuffer(emitter->positionBuffer);
	DestroyVertexBuffer(emitter->sizeBuffer);
	DestroyVertexBuffer(emitter->colorBuffer);

	DestroyTransferBuffer(emitter->positionTransferBuffer);
	DestroyTransferBuffer(emitter->sizeTransferBuffer);
	DestroyTransferBuffer(emitter->colorTransferBuffer);

	SDL_free(emitter->positions);
	SDL_free(emitter->sizes);
	SDL_free(emitter->colors);
	SDL_free(emitter->velocities);
	SDL_free(emitter->birthTimes);
	SDL_free(emitter->deathTimes);
}

static void SpawnParticle(ParticleEffect* effect, ParticleEmitter* emitter)
{
	SDL_assert(emitter->numParticles < emitter->maxParticles);

	int particleID = emitter->numParticles++;

	mat4 transform = ModelMatrix((Entity*)effect);

	emitter->positions[particleID] = (transform * vec4(emitter->startPosition, 1)).xyz;

	emitter->sizes[particleID] = vec2(mix(emitter->startSize, emitter->endSize, game->random.nextFloat()));

	emitter->colors[particleID] = emitter->startColor;

	emitter->velocities[particleID] = (transform * vec4(emitter->startVelocity + emitter->randomVelocity * game->random.nextVector3(-1, 1), 0)).xyz;

	float lifetime = mix(emitter->minLifetime, emitter->maxLifetime, game->random.nextFloat());
	emitter->birthTimes[particleID] = gameTime;
	emitter->deathTimes[particleID] = gameTime + lifetime;
}

static void KillParticle(ParticleEmitter* emitter, int id)
{
	int lastParticle = --emitter->numParticles;

	if (id != lastParticle)
	{
		emitter->positions[id] = emitter->positions[lastParticle];
		emitter->sizes[id] = emitter->sizes[lastParticle];
		emitter->colors[id] = emitter->colors[lastParticle];
		emitter->velocities[id] = emitter->velocities[lastParticle];
		emitter->birthTimes[id] = emitter->birthTimes[lastParticle];
		emitter->deathTimes[id] = emitter->deathTimes[lastParticle];
	}
}

static void UpdateParticleEmitter(ParticleEffect* effect, ParticleEmitter* emitter)
{
	for (int i = 0; i < emitter->numParticles; i++)
	{
		if (gameTime >= emitter->deathTimes[i])
			KillParticle(emitter, i--);
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		float progress = remap(gameTime, emitter->birthTimes[i], emitter->deathTimes[i], 0, 1);
		float size = mix(emitter->startSize, emitter->endSize, progress);
		emitter->sizes[i] = vec2(size);
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		float progress = remap(gameTime, emitter->birthTimes[i], emitter->deathTimes[i], 0, 1);
		vec4 color = mix(emitter->startColor, emitter->endColor, progress);
		emitter->colors[i] = color;
	}

	for (int i = 0; i < emitter->numParticles; i++)
		emitter->velocities[i] += 0.5f * emitter->gravity * deltaTime;

	for (int i = 0; i < emitter->numParticles; i++)
		emitter->positions[i] += emitter->velocities[i] * deltaTime;

	for (int i = 0; i < emitter->numParticles; i++)
		emitter->velocities[i] += 0.5f * emitter->gravity * deltaTime;

	float numSpawnsF = emitter->spawnRate * deltaTime + emitter->spawnRemainder;
	int numSpawns = (int)numSpawnsF;
	emitter->spawnRemainder = numSpawnsF - numSpawns;

	for (int i = 0; i < numSpawns; i++)
		SpawnParticle(effect, emitter);


	if (emitter->numParticles)
	{
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		void* mapped = MapTransferBuffer(emitter->positionTransferBuffer);
		SDL_memcpy(mapped, emitter->positions, emitter->numParticles * sizeof(vec3));
		UnmapTransferBuffer(emitter->positionTransferBuffer);

		mapped = MapTransferBuffer(emitter->sizeTransferBuffer);
		SDL_memcpy(mapped, emitter->sizes, emitter->numParticles * sizeof(vec2));
		UnmapTransferBuffer(emitter->sizeTransferBuffer);

		mapped = MapTransferBuffer(emitter->colorTransferBuffer);
		SDL_memcpy(mapped, emitter->colors, emitter->numParticles * sizeof(vec4));
		UnmapTransferBuffer(emitter->colorTransferBuffer);

		UpdateVertexBuffer(emitter->positionBuffer, 0, emitter->numParticles * sizeof(vec3), emitter->positionTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->sizeBuffer, 0, emitter->numParticles * sizeof(vec2), emitter->sizeTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->colorBuffer, 0, emitter->numParticles * sizeof(vec4), emitter->colorTransferBuffer->buffer, copyPass);

		SDL_EndGPUCopyPass(copyPass);
	}
}

static void RenderParticleEmitter(ParticleSystem* particles, ParticleEmitter* emitter)
{
	VertexBuffer* buffers[4];
	buffers[0] = particles->quad;
	buffers[1] = emitter->positionBuffer;
	buffers[2] = emitter->sizeBuffer;
	buffers[3] = emitter->colorBuffer;

	vec4 params = vec4(emitter->texture ? 1.0f : 0.0f, 0, 0, 0);

	RenderMesh(&game->renderer, buffers, 4, nullptr, 4, emitter->numParticles, {}, {}, &params, sizeof(params), &emitter->texture, &emitter->textureSampler, 1, emitter->shader, mat4::Identity);
}

void InitParticleEffect(ParticleEffect* effect, vec3 position)
{
	game->particles.effects.add(effect);

	InitEntity((Entity*)effect, ENTITY_TYPE_PARTICLE_EFFECT);

	effect->position = position;
}

void DestroyParticleEffect(ParticleEffect* effect)
{
	for (int i = 0; i < effect->numEmitters; i++)
	{
		DestroyParticleEmitter(&effect->emitters[i]);
	}

	game->particles.effects.remove(effect);
}

ParticleEmitter* AddEmitter(ParticleEffect* effect, bool additive, float spawnRate, float minLifetime, float maxLifetime)
{
	ParticleEmitter* emitter = &effect->emitters[effect->numEmitters++];
	InitParticleEmitter(emitter, additive, spawnRate, minLifetime, maxLifetime);
	return emitter;
}

void StopParticleEffect(ParticleEffect* effect)
{
	for (int i = 0; i < effect->numEmitters; i++)
	{
		effect->emitters[i].spawnRate = 0;
	}
}

void UpdateParticleEffect(ParticleEffect* effect)
{
	for (int i = 0; i < effect->numEmitters; i++)
	{
		UpdateParticleEmitter(effect, &effect->emitters[i]);
	}

	if (effect->destroyOnFinish)
	{
		bool hasFinished = true;
		for (int i = 0; i < effect->numEmitters; i++)
		{
			bool emitterHasFinished = effect->emitters[i].spawnRate == 0 && effect->emitters[i].numParticles == 0;
			hasFinished = hasFinished && emitterHasFinished;
		}

		if (hasFinished)
			effect->removed = true;
	}
}

void RenderParticleEffect(ParticleSystem* particles, ParticleEffect* effect)
{
	for (int i = 0; i < effect->numEmitters; i++)
	{
		RenderParticleEmitter(particles, &effect->emitters[i]);
	}
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
