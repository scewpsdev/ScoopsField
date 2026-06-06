#include "ParticleSystem.h"

#include "utils/TokenReader.h"


#include "ParticleLoader.cpp"


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
	instanceLayouts[2].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
	instanceLayouts[2].perInstance = true;

	instanceLayouts[3].numAttributes = 1;
	instanceLayouts[3].attributes[0].location = 4;
	instanceLayouts[3].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayouts[3].perInstance = true;

	instanceLayouts[4].numAttributes = 1;
	instanceLayouts[4].attributes[0].location = 5;
	instanceLayouts[4].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
	instanceLayouts[4].perInstance = true;
}

void InitParticleEmitter(ParticleEffect* effect, ParticleEmitter* emitter, bool additive, float spawnRate, float minLifetime, float maxLifetime, int burstCount)
{
	emitter->shader = additive ? game->particleAdditiveShader : game->particleShader;

	emitter->minLifetime = minLifetime;
	emitter->maxLifetime = maxLifetime;
	emitter->spawnRate = spawnRate;
	emitter->spawnRemainder = 0.0f;
	emitter->spawnShape = SPAWN_SHAPE_POINT;
	emitter->spawnRadius = 0.0f;
	emitter->spawnPoint0 = vec3(0);
	emitter->spawnPoint1 = vec3(0);
	emitter->spawnSize = vec3(0);
	emitter->follow = false;
	emitter->size = 0.1f;
	emitter->endSize = 0.1f;
	emitter->gravity = vec3(0);
	emitter->drag = 0;
	emitter->startPosition = vec3(0);
	emitter->startVelocity = vec3(0);
	emitter->randomDirection = 0;
	emitter->randomVelocity = 0;
	emitter->velocityNoise = 0;
	emitter->randomRotation = false;
	emitter->rotationSpeed = 0;
	emitter->randomRotationSpeed = 0;
	emitter->color = vec4(1);
	emitter->endColor = vec4(1);
	emitter->texture = nullptr;
	emitter->textureSampler = TEXTURE_SAMPLER_DEFAULT;
	emitter->atlasSize = ivec2(0);
	emitter->atlasFrameCount = 0;
	//emitter->isBurst = burstCount;
	//emitter->burstTime = 0;
	emitter->burstCount = burstCount;
	//emitter->burstDuration = 0;
	//emitter->hasBursted = false;

	int maxParticles = (int)SDL_ceilf((emitter->maxLifetime + 0.1f) * emitter->spawnRate) + burstCount;

	emitter->maxParticles = maxParticles;
	emitter->numParticles = 0;

	emitter->positions = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	emitter->sizes = (vec2*)SDL_malloc(maxParticles * sizeof(vec2));
	emitter->rotations = (float*)SDL_malloc(maxParticles * sizeof(float));
	emitter->velocities = (vec3*)SDL_malloc(maxParticles * sizeof(vec3));
	emitter->colors = (vec4*)SDL_malloc(maxParticles * sizeof(vec4));
	emitter->animations = (float*)SDL_malloc(maxParticles * sizeof(float));
	emitter->birthTimes = (float*)SDL_malloc(maxParticles * sizeof(float));
	emitter->deathTimes = (float*)SDL_malloc(maxParticles * sizeof(float));

	VertexBufferLayout instanceLayouts[5];
	InitParticleInstanceBufferLayouts(instanceLayouts);

	emitter->positionBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[0], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->sizeBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[1], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->rotationBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[2], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->colorBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[3], SDL_GPU_BUFFERUSAGE_VERTEX);
	emitter->animationBuffer = CreateVertexBuffer(maxParticles, &instanceLayouts[4], SDL_GPU_BUFFERUSAGE_VERTEX);

	emitter->positionTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec3), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->sizeTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec2), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->rotationTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(float), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->colorTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(vec4), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	emitter->animationTransferBuffer = CreateTransferBuffer(maxParticles * sizeof(float), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
}

void DestroyParticleEmitter(ParticleEmitter* emitter)
{
	DestroyVertexBuffer(emitter->positionBuffer);
	DestroyVertexBuffer(emitter->sizeBuffer);
	DestroyVertexBuffer(emitter->rotationBuffer);
	DestroyVertexBuffer(emitter->colorBuffer);
	DestroyVertexBuffer(emitter->animationBuffer);

	DestroyTransferBuffer(emitter->positionTransferBuffer);
	DestroyTransferBuffer(emitter->sizeTransferBuffer);
	DestroyTransferBuffer(emitter->rotationTransferBuffer);
	DestroyTransferBuffer(emitter->colorTransferBuffer);
	DestroyTransferBuffer(emitter->animationTransferBuffer);

	SDL_free(emitter->positions);
	SDL_free(emitter->sizes);
	SDL_free(emitter->rotations);
	SDL_free(emitter->velocities);
	SDL_free(emitter->colors);
	SDL_free(emitter->animations);
	SDL_free(emitter->birthTimes);
	SDL_free(emitter->deathTimes);
}

static vec3 GetSpawnPosition(ParticleEmitter* emitter)
{
	if (emitter->spawnShape == SPAWN_SHAPE_POINT)
		return vec3(0);
	else if (emitter->spawnShape == SPAWN_SHAPE_SPHERE)
	{
		float r = emitter->spawnRadius * SDL_powf(game->random.nextFloat(), 0.333333f);
		float theta = game->random.nextFloat() * 2 * PI;
		float phi = game->random.nextFloat() * PI;
		return vec3(r * SDL_sinf(phi) * SDL_cosf(theta), r * SDL_sinf(phi) * SDL_sinf(theta), r * SDL_cosf(phi));
	}
	else if (emitter->spawnShape == SPAWN_SHAPE_CIRCLE)
	{
		// sqrt the random number to get an even distribution in the circle
		float r = emitter->spawnRadius * SDL_sqrtf(game->random.nextFloat());
		float theta = game->random.nextFloat() * 2 * PI;
		return vec3(r * SDL_cosf(theta), 0, r * SDL_sinf(theta));
	}
	else if (emitter->spawnShape == SPAWN_SHAPE_BOX)
	{
		float x = (game->random.nextFloat() * 2 - 1) * emitter->spawnSize.x;
		float y = (game->random.nextFloat() * 2 - 1) * emitter->spawnSize.y;
		float z = (game->random.nextFloat() * 2 - 1) * emitter->spawnSize.z;
		vec3 position = vec3(x, y, z);
		if (emitter->spawnRadius > 0)
		{
			float r = emitter->spawnRadius * game->random.nextFloat();
			float theta = game->random.nextFloat() * 2 * PI;
			float phi = game->random.nextFloat() * PI;
			position += vec3(r * SDL_sinf(phi) * SDL_cosf(theta), r * SDL_sinf(phi) * SDL_sinf(theta), r * SDL_cosf(phi));
		}
		return position;
	}
	else if (emitter->spawnShape == SPAWN_SHAPE_LINE)
	{
		float t = game->random.nextFloat();
		vec3 position = mix(emitter->spawnPoint0, emitter->spawnPoint1, t);
		if (emitter->spawnRadius > 0)
		{
			float r = emitter->spawnRadius * game->random.nextFloat();
			float theta = game->random.nextFloat() * 2 * PI;
			float phi = game->random.nextFloat() * PI;
			position += vec3(r * SDL_sinf(phi) * SDL_cosf(theta), r * SDL_sinf(phi) * SDL_sinf(theta), r * SDL_cosf(phi));
		}
		return position;
	}
	else
	{
		SDL_assert(false);
		return vec3(0);
	}
}

static void SpawnParticle(ParticleEffect* effect, ParticleEmitter* emitter, vec3 effectVelocity, vec4 effectAngularVelocity, mat4 transform)
{
	SDL_assert(emitter->numParticles < emitter->maxParticles);

	int particleID = emitter->numParticles++;

	vec3 localPosition = emitter->startPosition + GetSpawnPosition(emitter);
	vec3 position = localPosition;
	if (!emitter->follow)
		position = (transform * vec4(position, 1)).xyz;
	emitter->positions[particleID] = position;

	emitter->sizes[particleID] = vec2(emitter->size);

	emitter->rotations[particleID] = emitter->randomRotation ? game->random.nextFloat() * 2 * PI : 0;

	emitter->colors[particleID] = emitter->color;

	emitter->animations[particleID] = 0;

	vec3 velocity = emitter->startVelocity;
	if (emitter->randomDirection)
		velocity = velocity.length() * game->random.randomDirection(velocity.normalized(), emitter->randomDirection, emitter->randomDirectionUniform);
	if (emitter->randomVelocity)
		velocity += velocity * emitter->randomVelocity * game->random.nextFloat(-1, 1);
	velocity = (transform * vec4(velocity, 0)).xyz;
	if (emitter->inheritVelocity)
		velocity += effectVelocity;
	if (emitter->inheritCentrifugal)
	{
		float rotationAngle = effectAngularVelocity.w;
		if (rotationAngle != 0)
		{
			vec3 rotationAxis = effectAngularVelocity.xyz;
			// to be exact, angular velocity would be w = angle / 2pi / t. but we would multiply by 2pi anyways for calculating the linear velocity (v = w * 2pi * r)
			vec3 fromCenter = localPosition;
			vec3 projectedCenter = rotationAxis * dot(rotationAxis, fromCenter);
			vec3 fromRotationAxis = localPosition - projectedCenter;
			vec3 centrifugalVelocity = rotationAngle * fromRotationAxis; // w * 2pi * r
			velocity += centrifugalVelocity;
		}
	}
	emitter->velocities[particleID] = velocity;

	float lifetime = mix(emitter->minLifetime, emitter->maxLifetime, game->random.nextFloat());
	// we subtract deltaTime here so that when a lot of particles get spawned after a long frame, their birthTimes reflect that and they get despawned soon enough.
	// otherwise we might run out of particle slots this frame, while these particles will only get despawned next frame.
	// deltaTime is the length of last frame, not the current frame. so the amount of newly spawned particles has a 1 frame delay.
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
		emitter->rotations[id] = emitter->rotations[lastParticle];
		emitter->velocities[id] = emitter->velocities[lastParticle];
		emitter->colors[id] = emitter->colors[lastParticle];
		emitter->animations[id] = emitter->animations[lastParticle];
		emitter->birthTimes[id] = emitter->birthTimes[lastParticle];
		emitter->deathTimes[id] = emitter->deathTimes[lastParticle];
	}
}

static void UpdateParticleEmitter(ParticleEffect* effect, ParticleEmitter* emitter, vec3 effectVelocity, vec4 effectAngularVelocity, mat4 transform)
{
	if (emitter->burstCount)
	{
		int numSpawns;

		if (emitter->burstDuration)
		{
			float numSpawnsF = emitter->burstCount / emitter->burstDuration * deltaTime + emitter->burstRemainder;
			numSpawns = (int)numSpawnsF;
			emitter->burstRemainder = numSpawnsF - numSpawns;
			emitter->burstDuration = max(emitter->burstDuration - deltaTime, 0.0f);
		}
		else
		{
			numSpawns = emitter->burstCount;
		}

		for (int i = 0; i < numSpawns; i++)
		{
			SpawnParticle(effect, emitter, effectVelocity, effectAngularVelocity, transform);
		}

		emitter->burstCount -= numSpawns;
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		if (gameTime >= emitter->deathTimes[i])
			KillParticle(emitter, i--);
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		float progress = remap(gameTime, emitter->birthTimes[i], emitter->deathTimes[i], 0, 1);
		float size = mix(emitter->size, emitter->endSize, progress);
		emitter->sizes[i] = vec2(size);
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		float rotationSpeed = emitter->rotationSpeed;
		if (emitter->randomRotationSpeed)
			rotationSpeed += (hash(i) % 0xFF / 255.0f * 2 - 1) * emitter->randomRotationSpeed;
		emitter->rotations[i] += rotationSpeed * deltaTime;
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		float progress = remap(gameTime, emitter->birthTimes[i], emitter->deathTimes[i], 0, 1);
		vec4 color = mix(emitter->color, emitter->endColor, progress);
		emitter->colors[i] = color;
	}

	if (emitter->atlasFrameCount)
	{
		for (int i = 0; i < emitter->numParticles; i++)
		{
			float progress = remap(gameTime, emitter->birthTimes[i], emitter->deathTimes[i], 0, 1);
			float animationFrame = progress * emitter->atlasFrameCount / (emitter->atlasSize.x * emitter->atlasSize.y);
			emitter->animations[i] = animationFrame;
		}
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		emitter->velocities[i] += 0.5f * emitter->gravity * deltaTime;

	}

	if (emitter->drag)
	{
		for (int i = 0; i < emitter->numParticles; i++)
		{
			vec3 dragForce = emitter->velocities[i] * emitter->velocities[i].length() * emitter->drag;
			emitter->velocities[i] += -dragForce * deltaTime;
		}
	}

	for (int i = 0; i < emitter->numParticles; i++)
	{
		emitter->positions[i] += emitter->velocities[i] * deltaTime;
	}

	if (emitter->velocityNoise)
	{
		for (int i = 0; i < emitter->numParticles; i++)
		{
			float t = gameTime + i;
			vec3 velocityNoise = vec3(Simplex1f(t), Simplex1f(100 + t), Simplex1f(200 + -t)).normalized();
			emitter->positions[i] += emitter->velocityNoise * velocityNoise * deltaTime;
		}
	}

	for (int i = 0; i < emitter->numParticles; i++)
		emitter->velocities[i] += 0.5f * emitter->gravity * deltaTime;

	float numSpawnsF = emitter->spawnRate * deltaTime + emitter->spawnRemainder;
	int numSpawns = (int)numSpawnsF;
	emitter->spawnRemainder = numSpawnsF - numSpawns;

	for (int i = 0; i < numSpawns; i++)
		SpawnParticle(effect, emitter, effectVelocity, effectAngularVelocity, transform);


	if (emitter->numParticles)
	{
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		void* mapped = MapTransferBuffer(emitter->positionTransferBuffer);
		SDL_memcpy(mapped, emitter->positions, emitter->numParticles * sizeof(vec3));
		UnmapTransferBuffer(emitter->positionTransferBuffer);

		mapped = MapTransferBuffer(emitter->sizeTransferBuffer);
		SDL_memcpy(mapped, emitter->sizes, emitter->numParticles * sizeof(vec2));
		UnmapTransferBuffer(emitter->sizeTransferBuffer);

		mapped = MapTransferBuffer(emitter->rotationTransferBuffer);
		SDL_memcpy(mapped, emitter->rotations, emitter->numParticles * sizeof(float));
		UnmapTransferBuffer(emitter->rotationTransferBuffer);

		mapped = MapTransferBuffer(emitter->colorTransferBuffer);
		SDL_memcpy(mapped, emitter->colors, emitter->numParticles * sizeof(vec4));
		UnmapTransferBuffer(emitter->colorTransferBuffer);

		mapped = MapTransferBuffer(emitter->animationTransferBuffer);
		SDL_memcpy(mapped, emitter->animations, emitter->numParticles * sizeof(float));
		UnmapTransferBuffer(emitter->animationTransferBuffer);

		UpdateVertexBuffer(emitter->positionBuffer, 0, emitter->numParticles * sizeof(vec3), emitter->positionTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->sizeBuffer, 0, emitter->numParticles * sizeof(vec2), emitter->sizeTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->rotationBuffer, 0, emitter->numParticles * sizeof(float), emitter->rotationTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->colorBuffer, 0, emitter->numParticles * sizeof(vec4), emitter->colorTransferBuffer->buffer, copyPass);
		UpdateVertexBuffer(emitter->animationBuffer, 0, emitter->numParticles * sizeof(float), emitter->animationTransferBuffer->buffer, copyPass);

		SDL_EndGPUCopyPass(copyPass);
	}
}

static void RenderParticleEmitter(ParticleSystem* particles, ParticleEmitter* emitter, mat4 transform)
{
	VertexBuffer* buffers[6];
	buffers[0] = particles->quad;
	buffers[1] = emitter->positionBuffer;
	buffers[2] = emitter->sizeBuffer;
	buffers[3] = emitter->rotationBuffer;
	buffers[4] = emitter->colorBuffer;
	buffers[5] = emitter->animationBuffer;

	vec4 params = vec4(emitter->texture ? 1.0f : 0.0f, emitter->atlasFrameCount ? 1.0f : 0.0f, (vec2)emitter->atlasSize);

	RenderMesh(&game->renderer, buffers, 6, nullptr, 4, emitter->numParticles, {}, {}, &params, sizeof(params), &emitter->texture, &emitter->textureSampler, 1, emitter->shader, emitter->follow ? transform : mat4::Identity);
}

void InitParticleEffect(ParticleEffect* effect, vec3 position, quat rotation)
{
	game->particles.effects.add(effect);

	InitEntity((Entity*)effect, ENTITY_TYPE_PARTICLE_EFFECT);

	effect->position = position;
	effect->rotation = rotation;

	effect->lastPosition = position;
	effect->lastRotation = rotation;
	effect->lastUpdate = gameTime;
}

void DestroyParticleEffect(ParticleEffect* effect)
{
	for (int i = 0; i < effect->numEmitters; i++)
	{
		DestroyParticleEmitter(&effect->emitters[i]);
	}

	game->particles.effects.remove(effect);
}

ParticleEmitter* AddEmitter(ParticleEffect* effect, bool additive, float spawnRate, float minLifetime, float maxLifetime, int burstCount)
{
	ParticleEmitter* emitter = &effect->emitters[effect->numEmitters++];
	InitParticleEmitter(effect, emitter, additive, spawnRate, minLifetime, maxLifetime, burstCount);
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
	vec3 dx = effect->position - effect->lastPosition;
	quat dr = effect->rotation * effect->lastRotation.conjugated();
	float dt = gameTime - effect->lastUpdate;
	effect->lastPosition = effect->position;
	effect->lastRotation = effect->rotation;
	effect->lastUpdate = gameTime;
	vec3 effectVelocity = dt > 0 ? dx / dt : vec3::Zero;
	vec4 effectAngularVelocity = dt > 0 ? dr.toAxisAngle() / vec4(1, 1, 1, dt) : vec4(1, 0, 0, 0);

	mat4 transform = ModelMatrix((Entity*)effect);
	for (int i = 0; i < effect->numEmitters; i++)
	{
		UpdateParticleEmitter(effect, &effect->emitters[i], effectVelocity, effectAngularVelocity, transform);
	}

	if (effect->destroyOnFinish)
	{
		bool hasFinished = true;
		for (int i = 0; i < effect->numEmitters; i++)
		{
			bool emitterHasFinished = effect->emitters[i].spawnRate == 0 && effect->emitters[i].numParticles == 0 && !effect->emitters[i].burstCount;
			hasFinished = hasFinished && emitterHasFinished;
		}

		if (hasFinished)
			effect->removed = true;
	}
}

void RenderParticleEffect(ParticleSystem* particles, ParticleEffect* effect)
{
	mat4 transform = ModelMatrix((Entity*)effect);
	for (int i = 0; i < effect->numEmitters; i++)
	{
		RenderParticleEmitter(particles, &effect->emitters[i], transform);
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
