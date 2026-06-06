
static void ReadBurst(TokenReader* reader, float* burstTime, int* burstCount, float* burstDuration)
{
	Next(reader, '{');

	while (!NextIsValue(reader, '}'))
	{
		Token name = Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, '='); // =
		if (CheckTokenValue(reader, &name, "time"))
			ReadFloat(reader, burstTime);
		else if (CheckTokenValue(reader, &name, "count"))
			ReadInteger(reader, burstCount);
		else if (CheckTokenValue(reader, &name, "duration"))
			ReadFloat(reader, burstDuration);
		else
		{
			SDL_assert(false);
			ReadValue(reader);
		}
	}

	Next(reader, '}');
}

static void ReadEmitter(TokenReader* reader, ParticleEffect* effect, const char* path)
{
	float lifetime;
	float randomLifetime;
	float size;
	float endSize;
	bool follow;
	float spawnRate;
	SpawnShape spawnShape;
	float spawnRadius = 0;
	vec3 spawnPoint0 = vec3(0);
	vec3 spawnPoint1 = vec3(0);
	vec3 spawnSize = vec3(0);
	vec3 startPosition;
	float gravity;
	float drag;

	vec3 startVelocity;
	float randomVelocity;
	float randomDirection;
	bool randomDirectionUniform;
	float velocityNoise;
	bool inheritVelocity;
	bool inheritCentrifugal;

	float startRotation;
	float rotationSpeed;
	bool randomRotation;
	float randomRotationSpeed;
	//bool inheritRotationSpeed;

	bool rotateForward;
	float stretchForward;

	char texture[256] = "";
	ivec2 atlasSize = ivec2(0);
	int frameCount = 0;
	bool randomFrame = false;
	bool linearFiltering = false;

	vec4 color;
	vec4 endColor;
	bool additive;
	float emissiveIntensity;

	bool isBurst = false;
	float burstTime = 0;
	int burstCount = 0;
	float burstDuration = 0;

	while (!NextIsValue(reader, '}'))
	{
		Token name = Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, '='); // =

		if (CheckTokenValue(reader, &name, "lifetime"))
			ReadFloat(reader, &lifetime);
		else if (CheckTokenValue(reader, &name, "size"))
		{
			ReadFloat(reader, &size);
			endSize = size;
		}
		else if (CheckTokenValue(reader, &name, "follow"))
			ReadBool(reader, &follow);
		else if (CheckTokenValue(reader, &name, "emissionRate"))
			ReadFloat(reader, &spawnRate);
		else if (CheckTokenValue(reader, &name, "spawnShape"))
		{
			Token value = Next(reader);
			if (CheckTokenValue(reader, &value, "Point"))
				spawnShape = SPAWN_SHAPE_POINT;
			else if (CheckTokenValue(reader, &value, "Sphere"))
				spawnShape = SPAWN_SHAPE_SPHERE;
			else if (CheckTokenValue(reader, &value, "Circle"))
				spawnShape = SPAWN_SHAPE_CIRCLE;
			else if (CheckTokenValue(reader, &value, "Box"))
				spawnShape = SPAWN_SHAPE_BOX;
			else if (CheckTokenValue(reader, &value, "Line"))
				spawnShape = SPAWN_SHAPE_LINE;
			else
			{
				SDL_assert(false);
			}
		}
		else if (CheckTokenValue(reader, &name, "spawnOffset"))
			ReadVec3(reader, &startPosition);
		else if (CheckTokenValue(reader, &name, "spawnRadius"))
			ReadFloat(reader, &spawnRadius);
		else if (CheckTokenValue(reader, &name, "spawnPoint0"))
			ReadVec3(reader, &spawnPoint0);
		else if (CheckTokenValue(reader, &name, "spawnPoint1"))
			ReadVec3(reader, &spawnPoint1);
		else if (CheckTokenValue(reader, &name, "spawnSize"))
			ReadVec3(reader, &spawnSize);
		else if (CheckTokenValue(reader, &name, "gravity"))
			ReadFloat(reader, &gravity);
		else if (CheckTokenValue(reader, &name, "drag"))
			ReadFloat(reader, &drag);
		else if (CheckTokenValue(reader, &name, "startVelocity"))
			ReadVec3(reader, &startVelocity);
		else if (CheckTokenValue(reader, &name, "randomVelocity"))
			ReadFloat(reader, &randomVelocity);
		else if (CheckTokenValue(reader, &name, "randomDirection"))
			ReadFloat(reader, &randomDirection);
		else if (CheckTokenValue(reader, &name, "randomDirectionUniform"))
			ReadBool(reader, &randomDirectionUniform);
		else if (CheckTokenValue(reader, &name, "startRotation"))
			ReadFloat(reader, &startRotation);
		else if (CheckTokenValue(reader, &name, "rotationSpeed"))
			ReadFloat(reader, &rotationSpeed);
		else if (CheckTokenValue(reader, &name, "applyEntityVelocity"))
			ReadBool(reader, &inheritVelocity);
		else if (CheckTokenValue(reader, &name, "applyCentrifugalForce"))
			ReadBool(reader, &inheritCentrifugal);
		else if (CheckTokenValue(reader, &name, "rotateAlongMovement"))
			ReadBool(reader, &rotateForward);
		else if (CheckTokenValue(reader, &name, "movementStretch"))
			ReadFloat(reader, &stretchForward);
		else if (CheckTokenValue(reader, &name, "textureAtlas"))
			ReadString(reader, texture, 256);
		else if (CheckTokenValue(reader, &name, "atlasSize"))
			ReadIVec2(reader, &atlasSize);
		else if (CheckTokenValue(reader, &name, "numFrames"))
			ReadInteger(reader, &frameCount);
		else if (CheckTokenValue(reader, &name, "randomFrame"))
			ReadBool(reader, &randomFrame);
		else if (CheckTokenValue(reader, &name, "linearFiltering"))
			ReadBool(reader, &linearFiltering);
		else if (CheckTokenValue(reader, &name, "color"))
		{
			ReadVec4(reader, &color);
			endColor = color;
		}
		else if (CheckTokenValue(reader, &name, "additive"))
			ReadBool(reader, &additive);
		else if (CheckTokenValue(reader, &name, "emissiveIntensity"))
			ReadFloat(reader, &emissiveIntensity);
		else if (CheckTokenValue(reader, &name, "randomRotation"))
			ReadBool(reader, &randomRotation);
		else if (CheckTokenValue(reader, &name, "randomRotationSpeed"))
			ReadFloat(reader, &randomRotationSpeed);
		else if (CheckTokenValue(reader, &name, "randomLifetime"))
			ReadFloat(reader, &randomLifetime);
		else if (CheckTokenValue(reader, &name, "velocityNoise"))
			ReadFloat(reader, &velocityNoise);
		else if (CheckTokenValue(reader, &name, "sizeAnim"))
		{
			vec3 sizeAnim;
			ReadVec3(reader, &sizeAnim);
			size = sizeAnim.x;
			endSize = sizeAnim.z;
		}
		else if (CheckTokenValue(reader, &name, "colorAnim0"))
			ReadVec4(reader, &color);
		else if (CheckTokenValue(reader, &name, "colorAnim1"))
			ReadVec4(reader, &endColor);
		else if (CheckTokenValue(reader, &name, "colorAnim2"))
			ReadVec4(reader, &endColor);
		else if (CheckTokenValue(reader, &name, "bursts"))
		{
			Next(reader, '['); // [

			bool hasNext = !NextIsValue(reader, ']');
			while (hasNext)
			{
				isBurst = true;
				ReadBurst(reader, &burstTime, &burstCount, &burstDuration);

				hasNext = NextIsValue(reader, ',');
				if (hasNext)
					Next(reader, ',');
			}

			Next(reader, ']'); // ]
		}
		else
		{
			const char* tokenString = &reader->data[name.start];
			SDL_assert(false);
			ReadValue(reader);
		}
	}

	ParticleEmitter* emitter = AddEmitter(effect, additive, spawnRate, lifetime - randomLifetime * lifetime, lifetime + randomLifetime * lifetime, burstCount);

	emitter->size = size;
	emitter->endSize = endSize;
	emitter->follow = follow;
	emitter->spawnRate = spawnRate;
	emitter->spawnShape = spawnShape;
	emitter->spawnRadius = spawnRadius;
	emitter->spawnPoint0 = spawnPoint0;
	emitter->spawnPoint1 = spawnPoint1;
	emitter->spawnSize = spawnSize;
	emitter->startPosition = startPosition;
	emitter->gravity = vec3(0, gravity, 0);
	emitter->drag = drag;

	emitter->startVelocity = startVelocity;
	emitter->randomVelocity = randomVelocity;
	emitter->randomDirection = randomDirection;
	emitter->randomDirectionUniform = randomDirectionUniform;
	emitter->velocityNoise = velocityNoise;
	emitter->inheritVelocity = inheritVelocity;
	emitter->inheritCentrifugal = inheritCentrifugal;

	//emitter->startRotation = startRotation;
	emitter->randomRotation = randomRotation;
	emitter->rotationSpeed = rotationSpeed;
	emitter->randomRotationSpeed = randomRotationSpeed;
	//emitter->inheritRotationSpeed = inheritRotationSpeed;

	//emitter->rotateForward = rotateForward;
	//emitter->stretchForward = stretchForward;

	if (texture[0])
	{
		char fullPath[256] = "";
		GetAbsolutePath(fullPath, 256, texture, path);
		emitter->texture = GetTexture(fullPath);
	}
	emitter->atlasSize = atlasSize;
	emitter->atlasFrameCount = frameCount;
	//emitter->randomFrame = randomFrame;
	emitter->textureSampler = linearFiltering ? TEXTURE_SAMPLER_LINEAR : TEXTURE_SAMPLER_DEFAULT;

	emitter->color = SRGBToLinear(color) * vec4(vec3(emissiveIntensity ? emissiveIntensity : 1), 1);
	emitter->endColor = SRGBToLinear(endColor) * vec4(vec3(emissiveIntensity ? emissiveIntensity : 1), 1);
	//emitter->emissiveIntensity = emissiveIntensity;

	//emitter->isBurst = isBurst;
	//emitter->burstTime = burstTime;
	//emitter->burstCount = burstCount;
	emitter->burstDuration = burstDuration;
}

static void ReadEntity(TokenReader* reader, ParticleEffect* effect, const char* path)
{
	while (Peek(reader).type == TOKEN_TYPE_IDENTIFIER)
	{
		Token name = Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, '='); // =
		if (CheckTokenValue(reader, &name, "particles"))
		{
			Next(reader, '['); // [

			bool hasNext = NextIsValue(reader, '{');
			while (hasNext)
			{
				Next(reader, '{'); // {

				ReadEmitter(reader, effect, path);

				Next(reader, '}'); // }

				hasNext = NextIsValue(reader, ',');
				if (hasNext)
					Next(reader, ','); // ,
			}

			Next(reader, ']'); // ]
		}
		else
		{
			ReadValue(reader);
		}
	}
}

void LoadParticleEffect(ParticleEffect* effect, const char* path, vec3 position, quat rotation)
{
	InitParticleEffect(effect, position, rotation);

	char fullPath[256];
	SDL_snprintf(fullPath, 256, "res/%s.bin", path);

	size_t fileSize;
	if (void* data = SDL_LoadFile(fullPath, &fileSize))
	{
		TokenReader reader = {};
		InitTokenReader(&reader, (char*)data, (int)fileSize);

		Next(&reader, TOKEN_TYPE_IDENTIFIER); // entities
		Next(&reader, '='); // =
		Next(&reader, '['); // [
		Next(&reader, '{'); // {

		ReadEntity(&reader, effect, path);

		Next(&reader, '}'); // }
		Next(&reader, ']'); // ]

		SDL_free(data);
	}
}
